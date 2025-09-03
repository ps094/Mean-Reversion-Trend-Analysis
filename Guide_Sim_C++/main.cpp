#include<iostream>
#include "Utilities.h"
#include "Strategy_Simulator.h"

using std::cout;
using std::cin;
using std::endl;

int main() {

    std::map<std::string, std::tuple<float, float, int>> range_parameters;
    std::map<std::string, std::string> string_parameters;

//    range_parameters["position_parameters"] = std::make_tuple(0.02f, 0.01f, 22);
//    range_parameters["move_parameters"] = std::make_tuple(0.17f, 0.01f, 42);
//    range_parameters["profit_parameters"] = std::make_tuple(0.05f, 0.01f, 24);
    range_parameters["position_parameters"] = std::make_tuple(0.01f, 0.02f, 18);
    range_parameters["move_parameters"] = std::make_tuple(0.04f, 0.02f, 29);
    range_parameters["profit_parameters"] = std::make_tuple(0.01f, 0.02f, 21);

    vector<float> position_ratio = {0.5f, 0.6f, 2.0f / 3.0f};

    string_parameters["underlying"] = "NIFTY";
    string_parameters["start_date"] = "2024-01-17"; //last trading day from where you want previous close
    string_parameters["end_date"] = "2024-01-19";
    string_parameters["previous_date"] = "2024-01-17"; //last trading day from where you want to import params

    map<string, vector<string>> multipliers;
    //position_parameter, abstinence_parameter, straight_abstinence_parameter, straight_reversal_parameter
    multipliers["position_multipliers"].push_back("1/1|3/2|11/10|1/3");
    multipliers["position_multipliers"].push_back("1/1|3/2|11/10|1/2");

    //take_profit_parameter, hedge_manage_parameter, stop_loss_parameter
    multipliers["profit_multipliers"].push_back("1/1|1/1|3/2");
    multipliers["profit_multipliers"].push_back("1/1|1/1|2/1");
    multipliers["profit_multipliers"].push_back("1/1|1/2|2/1");
    multipliers["profit_multipliers"].push_back("1/1|3/2|3/1");

    //extreme_move_parameter, directional_move_parameter, initial_move_parameter
    multipliers["move_multipliers"].push_back("5/4|1/1|3/2");
    multipliers["move_multipliers"].push_back("11/10|1/1|3/2");

    Strategy_Simulator *Sim = new Strategy_Simulator(range_parameters, multipliers, string_parameters, position_ratio);
    Sim->Start();
    delete Sim;
    return 0;
}
