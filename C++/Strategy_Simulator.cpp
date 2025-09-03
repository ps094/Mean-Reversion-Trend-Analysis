//
// Created by Praneet Shaw on 3/11/23.
//
#ifndef TEST_STRATEGY_SIMULATOR_CPP
#define TEST_STRATEGY_SIMULATOR_CPP

#include "Strategy_Simulator.h"

Strategy_Simulator::Strategy_Simulator(const map<string, tuple<float, float, int>> &range_parameters,
                                       const map<string, vector<string>> &multipliers_,
                                       const map<string, string> &string_parameters, const vector<float> &p_ratio) {
    underlying = string_parameters.at("underlying");
    start_date = string_parameters.at("start_date");
    end_date = string_parameters.at("end_date");
    previous_date = string_parameters.at("previous_date");
    position_ratios = p_ratio;

    std::cout << std::endl << "Position Parameters -> ";
    for (int i = 0; i < std::get<2>(range_parameters.at("position_parameters")); i++) {
        position_parameters.push_back(std::get<0>(range_parameters.at("position_parameters")) +
                                      (float) i * std::get<1>(range_parameters.at("position_parameters")));
        std::cout << position_parameters.back() << " , ";
    }

    std::cout << std::endl << "Move Parameters -> ";
    for (int i = 0; i < std::get<2>(range_parameters.at("move_parameters")); i++) {
        move_parameters.push_back(std::get<0>(range_parameters.at("move_parameters")) +
                                  (float) i * std::get<1>(range_parameters.at("move_parameters")));
        std::cout << move_parameters.back() << " , ";
    }

    std::cout << std::endl << "Profit Parameters -> ";
    for (int i = 0; i < std::get<2>(range_parameters.at("profit_parameters")); i++) {
        profit_parameters.push_back(std::get<0>(range_parameters.at("profit_parameters")) +
                                    (float) i * std::get<1>(range_parameters.at("profit_parameters")));
        std::cout << profit_parameters.back() << " , ";
    }

    std::cout << std::endl << "Position Ratios -> ";
    for (auto &p: position_ratios) {
        std::cout << p << " , ";
    }

    multipliers = multipliers_;
    Get_IV();
    Get_Days_To_Expiry();

    for (auto &i: multipliers) {
        std::cout << std::endl << i.first << " -> ";
        for (auto &j: i.second) { std::cout << j << " ; "; }
    }
    std::cout << std::endl << "Simulator Created for " << underlying << " from " << start_date << " to " << end_date;
}

void Strategy_Simulator::Get_IV() {
    Mongo_Adapter *DB = new Mongo_Adapter(config.DB_Server.c_str(), config.Data_DB.c_str(),
                                          config.Bhavcopy_Collection.c_str());
    DataFrame *DF = DB->get_document("Underlying", underlying.c_str(), config.Bhavcopy_Column_Name,
                                     config.Bhavcopy_Data_Types, config.Bhavcopy_Data_Count,
                                     config.Bhavcopy_Ignore_First);
    DF->get(IV);
    delete DB;
    delete DF;
}

void Strategy_Simulator::Get_Days_To_Expiry() {
    Mongo_Adapter *DB = new Mongo_Adapter(config.DB_Server.c_str(), config.Data_DB.c_str(),
                                          config.Days_To_Expiry_Collection.c_str());
    DataFrame *DF = DB->get_document("underlying", underlying.c_str(), config.Days_To_Expiry_Column_Name,
                                     config.Days_To_Expiry_Data_Types, config.Days_To_Expiry_Data_Count,
                                     config.Days_To_Expiry_Ignore_First);
    DF->get(Days_To_Expiry);
    delete DB;
    delete DF;
}

Strategy_Simulator::~Strategy_Simulator() {
    if (Solvers.size() > 0) { for (auto &s: Solvers) { delete s; }}
    std::cout << std::endl << "Simulator Destroyed";
}

void Strategy_Simulator::Get_Prices(const std::string &date) {
    prices.clear();
    prices.reserve(375);
    DOHLCI price_temp;

    Mongo_Adapter *DB = new Mongo_Adapter(config.DB_Server.c_str(), config.Data_DB.c_str(),
                                          (underlying + config.OHLC_Collection_Postfix).c_str());
    DataFrame *DF = DB->get_document(config.OHLC_Column_Name[0].c_str(), date.c_str(), config.OHLC_Column_Name,
                                     config.OHLC_Data_Types,
                                     config.OHLC_Data_Count, config.OHLC_Ignore_First);

    if (DF->row_count > 0) {
        for (int i = 0; i < DF->row_count; i++) {
            DF->get(i, price_temp);
            prices.push_back(price_temp);
        }
    }
    delete DB;
    delete DF;
}

vector<Simulation_Results>
Strategy_Simulator::Child_Thread_Simulator(Strategy_Parameters Params, string date, int numerical_location,
                                           tuple<int, int> expiries) {

    vector<Simulation_Results> Sim;
    Sim.reserve(config.child_sim_size);
    for (auto &p_param: position_parameters) {
        for (auto &m_param: move_parameters) {
            for (auto &pr_param: profit_parameters) {
                Sim.emplace_back(Simulation_Results());
                Params.Fill_Params(p_param, m_param, pr_param);
                Run_Sim(String_To_Time_T(date), Params, Sim.back(), expiries);
                Sim.back().Package(Params, underlying, date, numerical_location);
            }
        }
    }

    Solvers[numerical_location]->Infuse_Simulation_Results(Sim);
    Numerical_Sim_Results num_sim = Solvers[numerical_location]->Optimize();
    Solvers[numerical_location]->Update_Metadata(num_sim);
    vector<Simulation_Results> final_sim = {Sim[num_sim.position]};
    final_sim.back().num_sim = num_sim;
    return final_sim;
}

void Strategy_Simulator::Parent_Thread_Simulator(const string &date, const tuple<float, float> &vols,
                                                 const float &previous_close, const tuple<int, int> &expiries) {

    Parent_Sim.clear();

    Strategy_Parameters Parent_Params;
    vector<future<vector<Simulation_Results>>> threads;
    vector<Simulation_Results> child_sim;

    threads.reserve(config.thread_size);
    vector<string> position_multipliers = multipliers["position_multipliers"];
    vector<string> move_multipliers = multipliers["move_multipliers"];
    vector<string> profit_multipliers = multipliers["profit_multipliers"];
    threads.reserve(position_multipliers.size() * move_multipliers.size() * profit_multipliers.size());

    int numerical_location = 0;

//    bool test_=true;
//    vector<Simulation_Results> test_param;
//    if (test_){test_param.reserve(config.parent_sim_size);}

    for (auto &p_multiplier: position_multipliers) {
        for (auto &m_multiplier: move_multipliers) {
            for (auto &pr_multiplier: profit_multipliers) {
                for (auto week_type: week_param) {
                    for (auto &strategy_type: strategy_type_param) {
                        for (auto &strategy_version: strategy_version_param) {
                            for (auto &p_ratio_param: position_ratios) {

                                //if(test_ & (numerical_location<207 || numerical_location>207)){numerical_location++;continue;}

                                Parent_Params.Initialize(week_type, strategy_type, strategy_version, p_ratio_param,
                                                         p_multiplier, m_multiplier, pr_multiplier, vols,
                                                         previous_close);
                                threads.push_back(std::async(&Strategy_Simulator::Child_Thread_Simulator, this,
                                                             Parent_Params, date, numerical_location, expiries));
                                /*
                                if(!test_){threads.push_back(std::async(&Strategy_Simulator::Child_Thread_Simulator, this,
                                                             Parent_Params, date, numerical_location, expiries));
                                } else {
                                    std::cout<<numerical_location<<std::endl;
                                    test_param.push_back(Child_Thread_Simulator(Parent_Params,date,numerical_location,expiries)[0]);
                                }
                                */
                                numerical_location++;
                            }
                        }
                    }
                }
            }
        }
    }

    Parent_Sim.reserve(config.parent_sim_size);
    child_sim.reserve(config.child_sim_size);

    /*
    if(!test_){
        for (auto &thread: threads) {
            child_sim = thread.get();
            Parent_Sim.push_back(child_sim[0]);
        }
    }
    else{
        std::cout<<std::endl<<"Completed";
        for(int i =0; i<config.parent_sim_size;i++)
        {
            if(i>test_param.size()-1){Parent_Sim.push_back(test_param.back());}
            else {Parent_Sim.push_back(test_param[i]);}
        }
    }*/

    for (auto &thread: threads) {
        child_sim = thread.get();
        Parent_Sim.push_back(child_sim[0]);
    }

    std::cout << std::endl << "Date: " << date << "; Sim_Size: " << Parent_Sim.size() << ";";
}

void Strategy_Simulator::Run_Sim(const time_t &date, const Strategy_Parameters &strat, Simulation_Results &sim,
                                 const tuple<int, int> &expiries) const {
    Directional_Strategy_Manager DSM(date, strat);
    sim = DSM.test(prices, strat.current_week, strat.strike_offset, strat.sim_vol, expiries);
}

void Strategy_Simulator::Start() {
    std::string date = start_date;
    float previous_close;
    bool yesterday_flag = false;
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    while (date != end_date) {
        Get_Prices(date);
        if (!yesterday_flag && prices.size() > 0) {
            yesterday_flag = true;
            previous_close = prices.back().close;
        } else if (prices.size() > 0) {
            if (prices.size() == 375) {
                if (Solvers.size() == 0) { Initialize_Solvers(); }
                start = std::chrono::high_resolution_clock::now();

                Parent_Thread_Simulator(date, IV.ivols[date], previous_close, Days_To_Expiry.days_to_expiry[date]);
                Sim_To_CSV(Parent_Sim, date, string("_sim_") + underlying);
                Bridge_Expiry_Params(Days_To_Expiry.days_to_expiry[date]);

                end = std::chrono::high_resolution_clock::now();
                time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                std::cout << std::endl << "Simulation Time: " << ((double) time.count() / 1000000000.0);
            }
            previous_close = prices.back().close;
        }
        date = get_next_date(date);
    }
}

void Strategy_Simulator::Initialize_Solvers() {

    vector<Yesterday_Params> y_params;
    Solvers.reserve(config.thread_size);
    y_params.reserve(config.thread_size);

    Mongo_Adapter *DB = new Mongo_Adapter(config.DB_Server.c_str(), config.Data_DB.c_str(),
                                          (underlying + "_" + config.Simulated_Parameter_Collection).c_str());
    DataFrame *DF = DB->get_document(config.Simulated_Parameters_Column_Name[0].c_str(), string(previous_date).c_str(),
                                     config.Simulated_Parameters_Column_Name,
                                     config.Simulated_Parameters_Data_Types, config.Simulated_Parameters_Data_Count,
                                     config.Simulated_Parameters_Ignore_First);
    DF->get(y_params);

    if (y_params.size() == config.thread_size) { std::cout << std::endl << "Importing params from DB"; }
    for (int i = 0; i < config.thread_size; i++) {
        Solvers.emplace_back(
                new Numerical_Solver(config, i + 1, position_parameters, move_parameters, profit_parameters));
        if (y_params.size() == config.thread_size) { Solvers.back()->Update_Yesterday_Params(y_params[i]); }
    }
    Bridge_Expiry_Params(Days_To_Expiry.days_to_expiry[previous_date]);

    delete DB;
    delete DF;
}

void Strategy_Simulator::Bridge_Expiry_Params(const tuple<int, int> &expiries) {
    if (std::get<0>(expiries) == 1) {
        std::cout << std::endl << "Importing params from previous expiry";
        for (int i = 0; i < Solvers.size(); i++) {
            Solvers[i]->Update_Expiry_Params(Solvers[i + 12]->Get_Yesterday_Params());
            if ((i + 1) % 12 == 0) { i += 12; }
        }
    }
}

#endif