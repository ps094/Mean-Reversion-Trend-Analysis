//
// Created by Praneet Shaw on 3/11/23.
//

#ifndef TEST_STRATEGY_SIMULATOR_H
#define TEST_STRATEGY_SIMULATOR_H

#include<iostream>
#include<vector>
#include<map>
#include<tuple>
#include<future>
#include<chrono>

#include "Utilities.h"
#include "Mongo_Adapter.h"
#include "DataFrame.h"
#include "Directional_Strategy_Manager.h"
#include "Numerical_Solver.h"

using std::map;
using std::string;
using std::tuple;
using std::vector;
using std::future;

class Strategy_Simulator {

private:
    const Config config; //config_file
    string underlying, start_date, end_date, previous_date; //start_date is used to check previous close, simulation continues up to and not incl. end_date
    vector<float> position_parameters, move_parameters, profit_parameters, position_ratios;
    map<string, vector<string>> multipliers; //position_multipliers, move_multipliers, profit_multipliers
    IVOL_Query IV;
    Expiry_Query Days_To_Expiry;
    vector<DOHLCI> prices;
    vector<Simulation_Results> Parent_Sim;
    vector<Numerical_Solver *> Solvers;

    //const parameters
    const vector<bool> week_param = {true, false};
    const vector<int> strategy_type_param = {1, 2};
    const vector<int> strategy_version_param = {1, 2};

    void Get_IV();

    void Get_Days_To_Expiry();

    void Parent_Thread_Simulator(const string &, const tuple<float, float> &, const float &, const tuple<int, int> &);

    void Get_Prices(const std::string &);

    [[nodiscard]] vector<Simulation_Results> Child_Thread_Simulator(Strategy_Parameters, string, int, tuple<int, int>);

    void Run_Sim(const time_t &, const Strategy_Parameters &, Simulation_Results &, const tuple<int, int> &) const;

    void Initialize_Solvers();

    void Bridge_Expiry_Params(const tuple<int, int> &);

public:

    Strategy_Simulator(const map<string, tuple<float, float, int>> &, const map<string, vector<string>> &,
                       const map<string, string> &, const vector<float> &);

    ~Strategy_Simulator();

    Strategy_Simulator(const Strategy_Simulator &other) = default;

    Strategy_Simulator &operator=(const Strategy_Simulator &other) = default;

    void Start();
};

#endif //TEST_STRATEGY_SIMULATOR_H
