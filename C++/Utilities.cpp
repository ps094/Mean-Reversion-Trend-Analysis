//
// Created by Praneet Shaw on 3/12/23.
//
#ifndef UTILITIES_CPP
#define UTILITIES_CPP

#include "Utilities.h"

//Stray_Helpers
std::string get_next_date(const std::string &date_today) {
    time_t t = String_To_Time_T(date_today) + 1.5 * 60 * 60 * 24;
    return Time_T_To_String(t);
}

std::string Time_T_To_String(const time_t date) {
    struct std::tm TM;
    char temp[20] = "Default";
    localtime_r(&date, &TM);
    strftime(temp, 20, "%F", &TM);
    return std::string(temp);
}

time_t String_To_Time_T(const std::string &date) {
    struct std::tm TM;
    std::istringstream iss(date + " 00:00:00");
    iss >> std::get_time(&TM, "%Y-%m-%d %H:%M:%S");
    return mktime(&TM);
}

void
Sim_To_CSV(const std::vector<Simulation_Results> &Parent_Sim, const std::string &filename, const std::string &suffix) {
    const Config config;
    std::vector<std::string> columns;
    std::ofstream myfile;

    std::string file = config.file_location + filename + suffix + ".csv";

    myfile.open(file.c_str());
    myfile << "date,underlying,strategy_variant,sim_number,pos_param,mov_param,prof_param,strike_offset,sim_vol,"
           << "drawdown_count,trade_count,capital,pnl,first_drawdown_pnl,second_drawdown_pnl,taylor_sensitivity_1,"
           << "taylor_sensitivity_2,taylor_sensitivity_3,pos_dev,mov_dev,prof_dev,pos_diff,mov_diff,prof_diff,"
           << "pmpr_diff,variant_pnl_mean,variant_positive_pnl_mean,variant_negative_pnl_mean,variant_profit_percent,"
           << "variant_pnl_stdev,variant_positive_pnl_stdev,variant_negative_pnl_stdev,variant_pnl_skew,"
           << "variant_positive_pnl_skew,variant_negative_pnl_skew,variant_pnl_kurtosis,variant_positive_pnl_kurtosis,"
           << "variant_negative_pnl_kurtosis,variant_fav_drawdown_percent,variant_ts1_pnl_corr,variant_ts2_pnl_corr,"
           << "variant_ts3_pnl_corr,variant_ts1_mean,variant_ts2_mean,variant_ts3_mean,variant_ts1_stdev,"
           << "variant_ts2_stdev,variant_ts3_stdev,variant_pdev_mean,variant_mdev_mean,variant_prdev_mean,"
           << "variant_pdev_stdev,variant_mdev_stdev,variant_prdev_stdev,variant_pnl_pdiff_corr,variant_pnl_mdiff_corr,"
           << "variant_pnl_prdiff_corr,variant_pnl_pmprdiff_corr,variant_ts1_pdiff_corr,variant_ts1_mdiff_corr,"
           << "variant_ts1_prdiff_corr,variant_ts1_pmprdiff_corr,variant_ts2_pdiff_corr,variant_ts2_mdiff_corr,"
           << "variant_ts2_prdiff_corr,variant_ts2_pmprdiff_corr,variant_ts3_pdiff_corr,variant_ts3_mdiff_corr,"
           << "variant_ts3_prdiff_corr,variant_ts3_pmprdiff_corr,variant_pdev_pdiff_corr,variant_pdev_mdiff_corr,"
           << "variant_pdev_prdiff_corr,variant_pdev_pmprdiff_corr,variant_mdev_pdiff_corr,variant_mdev_mdiff_corr,"
           << "variant_mdev_prdiff_corr,variant_mdev_pmprdiff_corr,variant_prdev_pdiff_corr,variant_prdev_mdiff_corr,"
           << "variant_prdev_prdiff_corr,variant_prdev_pmprdiff_corr,variant_ts1_pdev_corr,variant_ts1_mdev_corr,"
           << "variant_ts1_prdev_corr,variant_ts2_pdev_corr,variant_ts2_mdev_corr,variant_ts2_prdev_corr,"
           << "variant_ts3_pdev_corr,variant_ts3_mdev_corr,variant_ts3_prdev_corr\n";

    for (auto &sim: Parent_Sim) {
        myfile << sim.date << "," << sim.underlying << "," << sim.strategy_variant << "," << sim.num_sim.position << ","
               << sim.pos_param << "," << sim.mov_param << "," << sim.prof_param << "," << sim.strike_offset << ","
               << sim.sim_vol << "," << sim.drawdown_count << "," << sim.trade_count << "," << sim.capital << ","
               << sim.pnl << "," << sim.first_drawdown_pnl << "," << sim.second_drawdown_pnl << ","
               << sim.num_sim.taylor_sensitivity_1 << "," << sim.num_sim.taylor_sensitivity_2 << ","
               << sim.num_sim.taylor_sensitivity_3 << "," << sim.num_sim.pos_dev << "," << sim.num_sim.mov_dev << ","
               << sim.num_sim.prof_dev << "," << sim.num_sim.pos_diff << "," << sim.num_sim.mov_diff << ","
               << sim.num_sim.prof_diff << "," << sim.num_sim.pmpr_diff << "," << sim.num_sim.variant_pnl_mean << ","
               << sim.num_sim.variant_positive_pnl_mean << "," << sim.num_sim.variant_negative_pnl_mean << ","
               << sim.num_sim.variant_profit_percent << "," << sim.num_sim.variant_pnl_stdev << ","
               << sim.num_sim.variant_positive_pnl_stdev << "," << sim.num_sim.variant_negative_pnl_stdev << ","
               << sim.num_sim.variant_pnl_skew << "," << sim.num_sim.variant_positive_pnl_skew << ","
               << sim.num_sim.variant_negative_pnl_skew << "," << sim.num_sim.variant_pnl_kurtosis << ","
               << sim.num_sim.variant_positive_pnl_kurtosis << "," << sim.num_sim.variant_negative_pnl_kurtosis << ","
               << sim.num_sim.variant_fav_drawdown_percent << "," << sim.num_sim.variant_ts1_pnl_corr << ","
               << sim.num_sim.variant_ts2_pnl_corr << "," << sim.num_sim.variant_ts3_pnl_corr << ","
               << sim.num_sim.variant_ts1_mean << "," << sim.num_sim.variant_ts2_mean << ","
               << sim.num_sim.variant_ts3_mean << "," << sim.num_sim.variant_ts1_stdev << ","
               << sim.num_sim.variant_ts2_stdev << "," << sim.num_sim.variant_ts3_stdev << ","
               << sim.num_sim.variant_pdev_mean << "," << sim.num_sim.variant_mdev_mean << ","
               << sim.num_sim.variant_prdev_mean << "," << sim.num_sim.variant_pdev_stdev << ","
               << sim.num_sim.variant_mdev_stdev << "," << sim.num_sim.variant_prdev_stdev << ","
               << sim.num_sim.variant_pnl_pdiff_corr << "," << sim.num_sim.variant_pnl_mdiff_corr << ","
               << sim.num_sim.variant_pnl_prdiff_corr << "," << sim.num_sim.variant_pnl_pmprdiff_corr << ","
               << sim.num_sim.variant_ts1_pdiff_corr << "," << sim.num_sim.variant_ts1_mdiff_corr << ","
               << sim.num_sim.variant_ts1_prdiff_corr << "," << sim.num_sim.variant_ts1_pmprdiff_corr << ","
               << sim.num_sim.variant_ts2_pdiff_corr << "," << sim.num_sim.variant_ts2_mdiff_corr << ","
               << sim.num_sim.variant_ts2_prdiff_corr << "," << sim.num_sim.variant_ts2_pmprdiff_corr << ","
               << sim.num_sim.variant_ts3_pdiff_corr << "," << sim.num_sim.variant_ts3_mdiff_corr << ","
               << sim.num_sim.variant_ts3_prdiff_corr << "," << sim.num_sim.variant_ts3_pmprdiff_corr << ","
               << sim.num_sim.variant_pdev_pdiff_corr << "," << sim.num_sim.variant_pdev_mdiff_corr << ","
               << sim.num_sim.variant_pdev_prdiff_corr << "," << sim.num_sim.variant_pdev_pmprdiff_corr << ","
               << sim.num_sim.variant_mdev_pdiff_corr << "," << sim.num_sim.variant_mdev_mdiff_corr << ","
               << sim.num_sim.variant_mdev_prdiff_corr << "," << sim.num_sim.variant_mdev_pmprdiff_corr << ","
               << sim.num_sim.variant_prdev_pdiff_corr << "," << sim.num_sim.variant_prdev_mdiff_corr << ","
               << sim.num_sim.variant_prdev_prdiff_corr << "," << sim.num_sim.variant_prdev_pmprdiff_corr << ","
               << sim.num_sim.variant_ts1_pdev_corr << "," << sim.num_sim.variant_ts1_mdev_corr << ","
               << sim.num_sim.variant_ts1_prdev_corr << "," << sim.num_sim.variant_ts2_pdev_corr << ","
               << sim.num_sim.variant_ts2_mdev_corr << "," << sim.num_sim.variant_ts2_prdev_corr << ","
               << sim.num_sim.variant_ts3_pdev_corr << "," << sim.num_sim.variant_ts3_mdev_corr << ","
               << sim.num_sim.variant_ts3_prdev_corr << "\n";
    }
    myfile.close();
}

//DOHLCI
DOHLCI::DOHLCI() : date(946684800), open(0.0f), high(0.0f), low(0.0f), close(0.0f), batch_id(0) {}

void DOHLCI::Print() const {
    std::cout << std::endl << "date: " << date << ", batch_id: " << batch_id << ", open: " << open << ", high: "
              << high << ", low: " << low << ", close: " << close;
}

//IVOL_Query
void IVOL_Query::Print() const {
    for (auto &ivol: ivols)
        std::cout << std::endl << ivol.first << ": IV_Current -> " << std::get<0>((ivol.second)) << ", IV_Next -> "
                  << std::get<1>((ivol.second));
}

//Strategy_Parameters
Strategy_Parameters::Strategy_Parameters() : initial_time_parameter(10), strike_offset(0.1f) {}

void Strategy_Parameters::Initialize(const bool &curr_week, const int &strat_type, const int &strat_ver,
                                     const float &pos_ratio, const std::string &p_key, const std::string &m_key,
                                     const std::string &pr_key, const std::tuple<float, float> &vols,
                                     const float &prev_close) {
    //position_multipliers, move_multipliers, profit_multipliers
    std::map<std::string, std::tuple<float, float, float, float>> position_multipliers_map;
    std::map<std::string, std::tuple<float, float, float>> move_multipliers_map;
    std::map<std::string, std::tuple<float, float, float>> profit_multipliers_map;

    //position_parameter, abstinence_parameter, straight_abstinence_parameter, straight_reversal_parameter
    position_multipliers_map["1/1|3/2|11/10|1/3"] = std::make_tuple(1.0f, 1.5f, 1.1f, 1.0f / 3.0f);
    position_multipliers_map["1/1|3/2|11/10|1/2"] = std::make_tuple(1.0f, 1.5f, 1.1f, 1.0f / 2.0f);

    //extreme_move_parameter, directional_move_parameter, initial_move_parameter
    move_multipliers_map["5/4|1/1|3/2"] = std::make_tuple(1.25f, 1.0f, 1.5f);
    move_multipliers_map["11/10|1/1|3/2"] = std::make_tuple(1.1f, 1.0f, 1.5f);

    //take_profit_parameter, hedge_manage_parameter, stop_loss_parameter
    profit_multipliers_map["1/1|1/1|3/2"] = std::make_tuple(1.0f, 1.0f, 1.5f);
    profit_multipliers_map["1/1|1/1|2/1"] = std::make_tuple(1.0f, 1.0f, 2.0f);
    profit_multipliers_map["1/1|1/2|2/1"] = std::make_tuple(1.0f, 0.5f, 2.0f);
    profit_multipliers_map["1/1|3/2|3/1"] = std::make_tuple(1.0f, 1.5f, 3.0f);

    position_multipliers = position_multipliers_map[p_key];
    move_multipliers = move_multipliers_map[m_key];
    profit_multipliers = profit_multipliers_map[pr_key];
    previous_close = prev_close;

    current_week = curr_week;
    if (curr_week) { sim_vol = std::get<0>(vols); }
    else { sim_vol = std::get<1>(vols); }
    strategy_type = strat_type;
    strategy_version = strat_ver;
    position_ratio = pos_ratio;

    //fill simulation results requirements
    p_multiplier_string = p_key;
    m_multiplier_string = m_key;
    pr_multiplier_string = pr_key;
}

void Strategy_Parameters::Fill_Params(const float &p_param, const float &m_param, const float &pr_param) {

    position_parameter = p_param * std::get<0>(position_multipliers);
    abstinence_parameter = p_param * std::get<1>(position_multipliers);
    straight_abstinence_parameter = p_param * std::get<2>(position_multipliers);
    straight_reversal_parameter = p_param * std::get<3>(position_multipliers);

    extreme_move_parameter = m_param * std::get<0>(move_multipliers);
    directional_move_parameter = m_param * std::get<1>(move_multipliers);
    initial_move_parameter = m_param * std::get<2>(move_multipliers);

    take_profit_parameter = pr_param * std::get<0>(profit_multipliers);
    hedge_manage_parameter = pr_param * std::get<1>(profit_multipliers);
    stop_loss_parameter = pr_param * std::get<2>(profit_multipliers);

    //fill simulation results requirements
    pos_param = p_param;
    mov_param = m_param;
    prof_param = pr_param;
}

//Simulation_Results
Simulation_Results::Simulation_Results() : capital(100000.0f), pnl(0.0f), first_drawdown_pnl(0.0f),
                                           second_drawdown_pnl(0.0f), drawdown_count(0), trade_count(0) {}

void Simulation_Results::Print() const {
    std::cout << std::endl << "Capital: " << capital << ", PnL: " << pnl << ", Drawdown Count: " << drawdown_count
              << ", Trade Count: " << trade_count;
}

void Simulation_Results::Package(const Strategy_Parameters &Params, const std::string &Under, const std::string &Date,
                                 const int &strat_variant) {
    strategy_variant = strat_variant + 1;
    pos_param = Params.pos_param;
    mov_param = Params.mov_param;
    prof_param = Params.prof_param;
    strike_offset = Params.strike_offset;
    sim_vol = Params.sim_vol;
    date = Date;
    underlying = Under;
}


#endif
