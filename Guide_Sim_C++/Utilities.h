#ifndef UTILITIES_H
#define UTILITIES_H

#include<map>
#include<tuple>
#include<ctime>
#include<iostream>
#include<iomanip>
#include<string>
#include<sstream>
#include<tuple>
#include<map>
#include<vector>
#include <fstream>

struct Config {
    std::string DB_Server = "mongodb://localhost:27017/";
    std::string Data_DB = "Strategy";
    long int parent_sim_size = 4209408;
    int child_sim_size = 10962;
    short int thread_size = 384;
    std::string file_location = "/Users/praneetshaw/Desktop/Strategy/CFiles/Guides/";

    std::string OHLC_Collection_Postfix = "OHLC";
    std::vector<std::string> OHLC_Column_Name = {"date", "open", "high", "low", "close", "batch_id"};
    std::vector<std::string> OHLC_Data_Types = {"date", "float", "float", "float", "float", "int"};
    int OHLC_Data_Count = 6;
    bool OHLC_Ignore_First = true;

    std::string Bhavcopy_Collection = "Daily_Bhavcopy";
    std::vector<std::string> Bhavcopy_Column_Name = {"Date", "IV_Current", "IV_Next", "Underlying"};
    std::vector<std::string> Bhavcopy_Data_Types = {"string", "float", "float", "string"};
    int Bhavcopy_Data_Count = 4;
    bool Bhavcopy_Ignore_First = true;

    std::string Days_To_Expiry_Collection = "Days_To_Expiry";
    std::vector<std::string> Days_To_Expiry_Column_Name = {"date", "current_week", "next_week", "underlying"};
    std::vector<std::string> Days_To_Expiry_Data_Types = {"string", "int", "int", "string"};
    int Days_To_Expiry_Data_Count = 4;
    bool Days_To_Expiry_Ignore_First = true;

    std::string Test_Collection = "Test_DB";
    std::vector<std::string> Test_Column_Name = {"date", "underlying", "strategy_type", "strategy_version",
                                                 "current_week", "m_multiplier_string", "p_multiplier_string",
                                                 "pr_multiplier_string", "pos_param", "mov_param", "prof_param",
                                                 "pos_ratio", "strike_offset", "sim_vol", "drawdown_count",
                                                 "trade_count", "capital", "pnl", "first_drawdown_pnl",
                                                 "second_drawdown_pnl"};
    std::vector<std::string> Test_Data_Types = {"string", "string", "int", "int", "int", "string", "string", "string",
                                                "float", "float", "float", "float", "float", "float", "int", "int",
                                                "float", "float", "float", "float"};
    int Test_Data_Count = 20;
    bool Test_Ignore_First = true;

    std::string Simulated_Parameter_Collection = "Guide_Parameters"; //Postfix
    std::vector<std::string> Simulated_Parameters_Column_Name = {"date", "underlying", "strategy_variant",
                                                                 "sim_number", "pos_param",
                                                                 "mov_param", "prof_param", "strike_offset", "sim_vol",
                                                                 "drawdown_count", "trade_count", "capital", "pnl",
                                                                 "first_drawdown_pnl", "second_drawdown_pnl",
                                                                 "taylor_sensitivity_1", "taylor_sensitivity_2",
                                                                 "taylor_sensitivity_3", "pos_dev", "mov_dev",
                                                                 "prof_dev", "pos_diff", "mov_diff", "prof_diff",
                                                                 "pmpr_diff", "variant_pnl_mean",
                                                                 "variant_positive_pnl_mean",
                                                                 "variant_negative_pnl_mean",
                                                                 "variant_profit_percent", "variant_pnl_stdev",
                                                                 "variant_positive_pnl_stdev",
                                                                 "variant_negative_pnl_stdev", "variant_pnl_skew",
                                                                 "variant_positive_pnl_skew",
                                                                 "variant_negative_pnl_skew",
                                                                 "variant_pnl_kurtosis",
                                                                 "variant_positive_pnl_kurtosis",
                                                                 "variant_negative_pnl_kurtosis",
                                                                 "variant_fav_drawdown_percent", "variant_ts1_pnl_corr",
                                                                 "variant_ts2_pnl_corr", "variant_ts3_pnl_corr",
                                                                 "variant_ts1_mean", "variant_ts2_mean",
                                                                 "variant_ts3_mean", "variant_ts1_stdev",
                                                                 "variant_ts2_stdev", "variant_ts3_stdev",
                                                                 "variant_pdev_mean", "variant_mdev_mean",
                                                                 "variant_prdev_mean", "variant_pdev_stdev",
                                                                 "variant_mdev_stdev", "variant_prdev_stdev",
                                                                 "variant_pnl_pdiff_corr", "variant_pnl_mdiff_corr",
                                                                 "variant_pnl_prdiff_corr", "variant_pnl_pmprdiff_corr",
                                                                 "variant_ts1_pdiff_corr", "variant_ts1_mdiff_corr",
                                                                 "variant_ts1_prdiff_corr", "variant_ts1_pmprdiff_corr",
                                                                 "variant_ts2_pdiff_corr", "variant_ts2_mdiff_corr",
                                                                 "variant_ts2_prdiff_corr", "variant_ts2_pmprdiff_corr",
                                                                 "variant_ts3_pdiff_corr", "variant_ts3_mdiff_corr",
                                                                 "variant_ts3_prdiff_corr", "variant_ts3_pmprdiff_corr",
                                                                 "variant_pdev_pdiff_corr", "variant_pdev_mdiff_corr",
                                                                 "variant_pdev_prdiff_corr",
                                                                 "variant_pdev_pmprdiff_corr",
                                                                 "variant_mdev_pdiff_corr", "variant_mdev_mdiff_corr",
                                                                 "variant_mdev_prdiff_corr",
                                                                 "variant_mdev_pmprdiff_corr",
                                                                 "variant_prdev_pdiff_corr", "variant_prdev_mdiff_corr",
                                                                 "variant_prdev_prdiff_corr",
                                                                 "variant_prdev_pmprdiff_corr", "variant_ts1_pdev_corr",
                                                                 "variant_ts1_mdev_corr", "variant_ts1_prdev_corr",
                                                                 "variant_ts2_pdev_corr", "variant_ts2_mdev_corr",
                                                                 "variant_ts2_prdev_corr", "variant_ts3_pdev_corr",
                                                                 "variant_ts3_mdev_corr", "variant_ts3_prdev_corr"};
    std::vector<std::string> Simulated_Parameters_Data_Types = {"string", "string", "int", "int",
                                                                "float", "float", "float", "float",
                                                                "float", "int", "int", "float", "float", "float",
                                                                "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float",
                                                                "float", "float", "float", "float", "float", "float",
                                                                "float"};
    int Simulated_Parameters_Data_Count = 91;
    bool Simulated_Parameters_Ignore_First = true;

    std::string Strategy_Variant_Collection = "Strategy_Variants";
    std::vector<std::string> Strategy_Variant_Columns_Name = {"underlying", "strategy_variant",
                                                              "position_multiplier_string", "position_multiplier",
                                                              "abstinence_multiplier", "straight_abstinence_multiplier",
                                                              "straight_reversal_multiplier", "move_multiplier_string",
                                                              "extreme_move_multiplier", "directional_move_multiplier",
                                                              "initial_move_multiplier", "profit_multiplier_string",
                                                              "take_profit_multiplier", "hedge_manage_multiplier",
                                                              "stop_loss_multiplier", "week_parameter",
                                                              "strategy_type_parameter", "strategy_version_parameter",
                                                              "position_ratio"};
    std::vector<std::string> Strategy_Variant_Data_Types = {"string", "int", "string", "float", "float", "float",
                                                            "float",
                                                            "string", "float", "float", "float", "string", "float",
                                                            "float",
                                                            "float", "int", "int", "int", "float"};
    int Strategy_Variant_Data_Count = 19;
    bool Strategy_Variant_Ignore_First = true;

    //state_params
//    float pos_increment = 0.01f, mov_increment = 0.01f, prof_increment = 0.01f;
    float pos_increment = 0.02f, mov_increment = 0.02f, prof_increment = 0.02f;
};

struct DOHLCI {
    time_t date;
    float open, high, low, close;
    int batch_id;

    DOHLCI();

    void Print() const;
};

struct IVOL_Query {
    std::map<std::string, std::tuple<float, float>> ivols;

    void Print() const;
};

struct Expiry_Query {
    std::map<std::string, std::tuple<int, int>> days_to_expiry;
};

struct Strategy_Parameters {
    //core parameters used by DSM
    int strategy_type, strategy_version, initial_time_parameter;
    float previous_close, position_parameter, extreme_move_parameter, abstinence_parameter, straight_abstinence_parameter,
            straight_reversal_parameter, directional_move_parameter, initial_move_parameter, position_ratio,
            take_profit_parameter, hedge_manage_parameter, stop_loss_parameter, strike_offset, sim_vol;
    bool current_week;

    //param use to fill core params
    std::tuple<float, float, float, float> position_multipliers;
    std::tuple<float, float, float> move_multipliers;
    std::tuple<float, float, float> profit_multipliers;

    //params required for simulation results only
    std::string p_multiplier_string, m_multiplier_string, pr_multiplier_string;
    float pos_param, mov_param, prof_param;

    Strategy_Parameters();

    void Initialize(const bool &, const int &, const int &, const float &, const std::string &, const std::string &,
                    const std::string &, const std::tuple<float, float> &, const float &);

    void Fill_Params(const float &, const float &, const float &);
};

struct Numerical_Sim_Results {
    int position;
    float variant_pnl_mean, variant_positive_pnl_mean, variant_negative_pnl_mean, variant_profit_percent,
            variant_pnl_stdev, variant_positive_pnl_stdev, variant_negative_pnl_stdev, variant_pnl_skew,
            variant_positive_pnl_skew, variant_negative_pnl_skew, variant_pnl_kurtosis, variant_positive_pnl_kurtosis,
            variant_negative_pnl_kurtosis, variant_fav_drawdown_percent, variant_ts1_pnl_corr, variant_ts2_pnl_corr,
            variant_ts3_pnl_corr, variant_ts1_mean, variant_ts2_mean, variant_ts3_mean, variant_ts1_stdev,
            variant_ts2_stdev, variant_ts3_stdev, variant_pdev_mean, variant_mdev_mean, variant_prdev_mean,
            variant_pdev_stdev, variant_mdev_stdev, variant_prdev_stdev, taylor_sensitivity_1, taylor_sensitivity_2,
            taylor_sensitivity_3, pos_dev, mov_dev, prof_dev, pos_diff, mov_diff, prof_diff, pmpr_diff,
            variant_pnl_pdiff_corr, variant_pnl_mdiff_corr, variant_pnl_prdiff_corr, variant_pnl_pmprdiff_corr,
            variant_ts1_pdiff_corr, variant_ts1_mdiff_corr, variant_ts1_prdiff_corr, variant_ts1_pmprdiff_corr,
            variant_ts2_pdiff_corr, variant_ts2_mdiff_corr, variant_ts2_prdiff_corr, variant_ts2_pmprdiff_corr,
            variant_ts3_pdiff_corr, variant_ts3_mdiff_corr, variant_ts3_prdiff_corr, variant_ts3_pmprdiff_corr,
            variant_pdev_pdiff_corr, variant_pdev_mdiff_corr, variant_pdev_prdiff_corr, variant_pdev_pmprdiff_corr,
            variant_mdev_pdiff_corr, variant_mdev_mdiff_corr, variant_mdev_prdiff_corr, variant_mdev_pmprdiff_corr,
            variant_prdev_pdiff_corr, variant_prdev_mdiff_corr, variant_prdev_prdiff_corr, variant_prdev_pmprdiff_corr,
            variant_ts1_pdev_corr, variant_ts1_mdev_corr, variant_ts1_prdev_corr, variant_ts2_pdev_corr,
            variant_ts2_mdev_corr, variant_ts2_prdev_corr, variant_ts3_pdev_corr, variant_ts3_mdev_corr,
            variant_ts3_prdev_corr;

    Numerical_Sim_Results() : position(0), variant_pnl_mean(0.0f), variant_positive_pnl_mean(0.0f),
                              variant_negative_pnl_mean(0.0f), variant_profit_percent(0.0f), variant_pnl_stdev(0.0f),
                              variant_positive_pnl_stdev(0.0f), variant_negative_pnl_stdev(0.0f),
                              variant_pnl_skew(0.0f), variant_positive_pnl_skew(0.0f), variant_negative_pnl_skew(0.0f),
                              variant_pnl_kurtosis(0.0f), variant_positive_pnl_kurtosis(0.0f),
                              variant_negative_pnl_kurtosis(0.0f), variant_fav_drawdown_percent(0.0f),
                              variant_ts1_pnl_corr(0.0f), variant_ts2_pnl_corr(0.0f), variant_ts3_pnl_corr(0.0f),
                              variant_ts1_mean(0.0f), variant_ts2_mean(0.0f), variant_ts3_mean(0.0f),
                              variant_ts1_stdev(0.0f), variant_ts2_stdev(0.0f), variant_ts3_stdev(0.0f),
                              variant_pdev_mean(0.0f), variant_mdev_mean(0.0f), variant_prdev_mean(0.0f),
                              variant_pdev_stdev(0.0f), variant_mdev_stdev(0.0f), variant_prdev_stdev(0.0f),
                              taylor_sensitivity_1(0.0f), taylor_sensitivity_2(0.0f), taylor_sensitivity_3(0.0f),
                              pos_dev(0.0f), mov_dev(0.0f), prof_dev(0.0f), pos_diff(0.0f), mov_diff(0.0f),
                              prof_diff(0.0f), pmpr_diff(0.0f), variant_pnl_pdiff_corr(0.0f),
                              variant_pnl_mdiff_corr(0.0f), variant_pnl_prdiff_corr(0.0f),
                              variant_pnl_pmprdiff_corr(0.0f), variant_ts1_pdiff_corr(0.0f),
                              variant_ts1_mdiff_corr(0.0f), variant_ts1_prdiff_corr(0.0f),
                              variant_ts1_pmprdiff_corr(0.0f), variant_ts2_pdiff_corr(0.0f),
                              variant_ts2_mdiff_corr(0.0f), variant_ts2_prdiff_corr(0.0f),
                              variant_ts2_pmprdiff_corr(0.0f), variant_ts3_pdiff_corr(0.0f),
                              variant_ts3_mdiff_corr(0.0f), variant_ts3_prdiff_corr(0.0f),
                              variant_ts3_pmprdiff_corr(0.0f), variant_pdev_pdiff_corr(0.0f),
                              variant_pdev_mdiff_corr(0.0f), variant_pdev_prdiff_corr(0.0f),
                              variant_pdev_pmprdiff_corr(0.0f), variant_mdev_pdiff_corr(0.0f),
                              variant_mdev_mdiff_corr(0.0f), variant_mdev_prdiff_corr(0.0f),
                              variant_mdev_pmprdiff_corr(0.0f), variant_prdev_pdiff_corr(0.0f),
                              variant_prdev_mdiff_corr(0.0f), variant_prdev_prdiff_corr(0.0f),
                              variant_prdev_pmprdiff_corr(0.0f), variant_ts1_pdev_corr(0.0f),
                              variant_ts1_mdev_corr(0.0f), variant_ts1_prdev_corr(0.0f), variant_ts2_pdev_corr(0.0f),
                              variant_ts2_mdev_corr(0.0f), variant_ts2_prdev_corr(0.0f), variant_ts3_pdev_corr(0.0f),
                              variant_ts3_mdev_corr(0.0f), variant_ts3_prdev_corr(0.0f) {}


    Numerical_Sim_Results(const int &pos) : position(pos), variant_pnl_mean(0.0f), variant_positive_pnl_mean(0.0f),
                                            variant_negative_pnl_mean(0.0f), variant_profit_percent(0.0f),
                                            variant_pnl_stdev(0.0f), variant_positive_pnl_stdev(0.0f),
                                            variant_negative_pnl_stdev(0.0f), variant_pnl_skew(0.0f),
                                            variant_positive_pnl_skew(0.0f), variant_negative_pnl_skew(0.0f),
                                            variant_pnl_kurtosis(0.0f), variant_positive_pnl_kurtosis(0.0f),
                                            variant_negative_pnl_kurtosis(0.0f), variant_fav_drawdown_percent(0.0f),
                                            variant_ts1_pnl_corr(0.0f), variant_ts2_pnl_corr(0.0f),
                                            variant_ts3_pnl_corr(0.0f), variant_ts1_mean(0.0f), variant_ts2_mean(0.0f),
                                            variant_ts3_mean(0.0f), variant_ts1_stdev(0.0f), variant_ts2_stdev(0.0f),
                                            variant_ts3_stdev(0.0f), variant_pdev_mean(0.0f), variant_mdev_mean(0.0f),
                                            variant_prdev_mean(0.0f), variant_pdev_stdev(0.0f),
                                            variant_mdev_stdev(0.0f), variant_prdev_stdev(0.0f),
                                            taylor_sensitivity_1(0.0f), taylor_sensitivity_2(0.0f),
                                            taylor_sensitivity_3(0.0f), pos_dev(0.0f), mov_dev(0.0f), prof_dev(0.0f),
                                            pos_diff(0.0f), mov_diff(0.0f), prof_diff(0.0f), pmpr_diff(0.0f),
                                            variant_pnl_pdiff_corr(0.0f), variant_pnl_mdiff_corr(0.0f),
                                            variant_pnl_prdiff_corr(0.0f), variant_pnl_pmprdiff_corr(0.0f),
                                            variant_ts1_pdiff_corr(0.0f), variant_ts1_mdiff_corr(0.0f),
                                            variant_ts1_prdiff_corr(0.0f), variant_ts1_pmprdiff_corr(0.0f),
                                            variant_ts2_pdiff_corr(0.0f), variant_ts2_mdiff_corr(0.0f),
                                            variant_ts2_prdiff_corr(0.0f), variant_ts2_pmprdiff_corr(0.0f),
                                            variant_ts3_pdiff_corr(0.0f), variant_ts3_mdiff_corr(0.0f),
                                            variant_ts3_prdiff_corr(0.0f), variant_ts3_pmprdiff_corr(0.0f),
                                            variant_pdev_pdiff_corr(0.0f), variant_pdev_mdiff_corr(0.0f),
                                            variant_pdev_prdiff_corr(0.0f), variant_pdev_pmprdiff_corr(0.0f),
                                            variant_mdev_pdiff_corr(0.0f), variant_mdev_mdiff_corr(0.0f),
                                            variant_mdev_prdiff_corr(0.0f), variant_mdev_pmprdiff_corr(0.0f),
                                            variant_prdev_pdiff_corr(0.0f), variant_prdev_mdiff_corr(0.0f),
                                            variant_prdev_prdiff_corr(0.0f), variant_prdev_pmprdiff_corr(0.0f),
                                            variant_ts1_pdev_corr(0.0f), variant_ts1_mdev_corr(0.0f),
                                            variant_ts1_prdev_corr(0.0f), variant_ts2_pdev_corr(0.0f),
                                            variant_ts2_mdev_corr(0.0f), variant_ts2_prdev_corr(0.0f),
                                            variant_ts3_pdev_corr(0.0f), variant_ts3_mdev_corr(0.0f),
                                            variant_ts3_prdev_corr(0.0f) {}
};


struct Simulation_Results {
    float capital, pnl, first_drawdown_pnl, second_drawdown_pnl, pos_param, mov_param, prof_param, strike_offset, sim_vol;
    int drawdown_count, trade_count, strategy_variant;
    std::string date, underlying;
    Numerical_Sim_Results num_sim;

    Simulation_Results();

    Simulation_Results(const Simulation_Results &other) = default;

    Simulation_Results &operator=(const Simulation_Results &other) = default;

    void Print() const;

    void Package(const Strategy_Parameters &, const std::string &, const std::string &, const int &);
};

struct Numerical_Point {
    float pos_param, mov_param, prof_param, pnl, taylor_sensitivity_1, taylor_sensitivity_2, taylor_sensitivity_3;
    int drawdown_count;
    float *pos_dev, *mov_dev, *prof_dev, pos_diff, mov_diff, prof_diff, pmpr_diff;
    float LHD_Pos, RHD_Pos, LHD_Mov, RHD_Mov, LHD_Prof, RHD_Prof, D_Pos, D_Mov, D_Prof;
    float DD_Pos_Pos, DD_Mov_Pos, DD_Prof_Pos, DD_Mov_Mov, DD_Pos_Mov, DD_Prof_Mov, DD_Prof_Prof, DD_Pos_Prof, DD_Mov_Prof;
    float DDD_Pos_Pos_Pos, DDD_Mov_Pos_Pos, DDD_Prof_Pos_Pos, DDD_Pos_Mov_Pos, DDD_Mov_Mov_Pos, DDD_Prof_Mov_Pos,
            DDD_Pos_Prof_Pos, DDD_Mov_Prof_Pos, DDD_Prof_Prof_Pos, DDD_Pos_Mov_Mov, DDD_Mov_Mov_Mov, DDD_Prof_Mov_Mov,
            DDD_Pos_Pos_Mov, DDD_Mov_Pos_Mov, DDD_Prof_Pos_Mov, DDD_Pos_Prof_Mov, DDD_Mov_Prof_Mov, DDD_Prof_Prof_Mov,
            DDD_Pos_Prof_Prof, DDD_Mov_Prof_Prof, DDD_Prof_Prof_Prof, DDD_Pos_Pos_Prof, DDD_Mov_Pos_Prof, DDD_Prof_Pos_Prof,
            DDD_Pos_Mov_Prof, DDD_Mov_Mov_Prof, DDD_Prof_Mov_Prof;
    float DDDD_Pos_Pos_Pos_Pos, DDDD_Mov_Pos_Pos_Pos, DDDD_Prof_Pos_Pos_Pos,
            DDDD_Pos_Mov_Pos_Pos, DDDD_Mov_Mov_Pos_Pos, DDDD_Prof_Mov_Pos_Pos,
            DDDD_Pos_Prof_Pos_Pos, DDDD_Mov_Prof_Pos_Pos, DDDD_Prof_Prof_Pos_Pos,
            DDDD_Pos_Pos_Mov_Pos, DDDD_Mov_Pos_Mov_Pos, DDDD_Prof_Pos_Mov_Pos,
            DDDD_Pos_Mov_Mov_Pos, DDDD_Mov_Mov_Mov_Pos, DDDD_Prof_Mov_Mov_Pos,
            DDDD_Pos_Prof_Mov_Pos, DDDD_Mov_Prof_Mov_Pos, DDDD_Prof_Prof_Mov_Pos,
            DDDD_Pos_Pos_Prof_Pos, DDDD_Mov_Pos_Prof_Pos, DDDD_Prof_Pos_Prof_Pos,
            DDDD_Pos_Mov_Prof_Pos, DDDD_Mov_Mov_Prof_Pos, DDDD_Prof_Mov_Prof_Pos,
            DDDD_Pos_Prof_Prof_Pos, DDDD_Mov_Prof_Prof_Pos, DDDD_Prof_Prof_Prof_Pos,
            DDDD_Pos_Pos_Mov_Mov, DDDD_Mov_Pos_Mov_Mov, DDDD_Prof_Pos_Mov_Mov,
            DDDD_Pos_Mov_Mov_Mov, DDDD_Mov_Mov_Mov_Mov, DDDD_Prof_Mov_Mov_Mov,
            DDDD_Pos_Prof_Mov_Mov, DDDD_Mov_Prof_Mov_Mov, DDDD_Prof_Prof_Mov_Mov,
            DDDD_Pos_Pos_Pos_Mov, DDDD_Mov_Pos_Pos_Mov, DDDD_Prof_Pos_Pos_Mov,
            DDDD_Pos_Mov_Pos_Mov, DDDD_Mov_Mov_Pos_Mov, DDDD_Prof_Mov_Pos_Mov,
            DDDD_Pos_Prof_Pos_Mov, DDDD_Mov_Prof_Pos_Mov, DDDD_Prof_Prof_Pos_Mov,
            DDDD_Pos_Pos_Prof_Mov, DDDD_Mov_Pos_Prof_Mov, DDDD_Prof_Pos_Prof_Mov,
            DDDD_Pos_Mov_Prof_Mov, DDDD_Mov_Mov_Prof_Mov, DDDD_Prof_Mov_Prof_Mov,
            DDDD_Pos_Prof_Prof_Mov, DDDD_Mov_Prof_Prof_Mov, DDDD_Prof_Prof_Prof_Mov,
            DDDD_Pos_Pos_Prof_Prof, DDDD_Mov_Pos_Prof_Prof, DDDD_Prof_Pos_Prof_Prof,
            DDDD_Pos_Mov_Prof_Prof, DDDD_Mov_Mov_Prof_Prof, DDDD_Prof_Mov_Prof_Prof,
            DDDD_Pos_Prof_Prof_Prof, DDDD_Mov_Prof_Prof_Prof, DDDD_Prof_Prof_Prof_Prof,
            DDDD_Pos_Pos_Pos_Prof, DDDD_Mov_Pos_Pos_Prof, DDDD_Prof_Pos_Pos_Prof,
            DDDD_Pos_Mov_Pos_Prof, DDDD_Mov_Mov_Pos_Prof, DDDD_Prof_Mov_Pos_Prof,
            DDDD_Pos_Prof_Pos_Prof, DDDD_Mov_Prof_Pos_Prof, DDDD_Prof_Prof_Pos_Prof,
            DDDD_Pos_Pos_Mov_Prof, DDDD_Mov_Pos_Mov_Prof, DDDD_Prof_Pos_Mov_Prof,
            DDDD_Pos_Mov_Mov_Prof, DDDD_Mov_Mov_Mov_Prof, DDDD_Prof_Mov_Mov_Prof,
            DDDD_Pos_Prof_Mov_Prof, DDDD_Mov_Prof_Mov_Prof, DDDD_Prof_Prof_Mov_Prof;

    Numerical_Point(const float &p_param, const float &m_param, const float &pr_param) :
            pos_param(p_param), mov_param(m_param), prof_param(pr_param), pnl(0.0f), drawdown_count(0),
            pos_dev(nullptr), mov_dev(nullptr), prof_dev(nullptr), pos_diff(0.0f), mov_diff(0.0f), prof_diff(0.0f),
            pmpr_diff(0.0f), LHD_Pos(0.0f), RHD_Pos(0.0f), LHD_Mov(0.0f), RHD_Mov(0.0f), LHD_Prof(0.0f),
            RHD_Prof(0.0f), D_Pos(0.0f), D_Mov(0.0f), D_Prof(0.0f), DD_Pos_Pos(0.0f), DD_Mov_Pos(0.0f),
            DD_Prof_Pos(0.0f), DD_Mov_Mov(0.0f), DD_Pos_Mov(0.0f), DD_Prof_Mov(0.0f), DD_Prof_Prof(0.0f),
            DD_Pos_Prof(0.0f), DD_Mov_Prof(0.0f), DDD_Pos_Pos_Pos(0.0f), DDD_Mov_Pos_Pos(0.0f),
            DDD_Prof_Pos_Pos(0.0f), DDD_Pos_Mov_Pos(0.0f), DDD_Mov_Mov_Pos(0.0f), DDD_Prof_Mov_Pos(0.0f),
            DDD_Pos_Prof_Pos(0.0f), DDD_Mov_Prof_Pos(0.0f), DDD_Prof_Prof_Pos(0.0f), DDD_Pos_Mov_Mov(0.0f),
            DDD_Mov_Mov_Mov(0.0f), DDD_Prof_Mov_Mov(0.0f), DDD_Pos_Pos_Mov(0.0f), DDD_Mov_Pos_Mov(0.0f),
            DDD_Prof_Pos_Mov(0.0f), DDD_Pos_Prof_Mov(0.0f), DDD_Mov_Prof_Mov(0.0f), DDD_Prof_Prof_Mov(0.0f),
            DDD_Pos_Prof_Prof(0.0f), DDD_Mov_Prof_Prof(0.0f), DDD_Prof_Prof_Prof(0.0f), DDD_Pos_Pos_Prof(0.0f),
            DDD_Mov_Pos_Prof(0.0f), DDD_Prof_Pos_Prof(0.0f), DDD_Pos_Mov_Prof(0.0f), DDD_Mov_Mov_Prof(0.0f),
            DDD_Prof_Mov_Prof(0.0f), DDDD_Pos_Pos_Pos_Pos(0.0f), DDDD_Mov_Pos_Pos_Pos(0.0f),
            DDDD_Prof_Pos_Pos_Pos(0.0f), DDDD_Pos_Mov_Pos_Pos(0.0f), DDDD_Mov_Mov_Pos_Pos(0.0f),
            DDDD_Prof_Mov_Pos_Pos(0.0f), DDDD_Pos_Prof_Pos_Pos(0.0f), DDDD_Mov_Prof_Pos_Pos(0.0f),
            DDDD_Prof_Prof_Pos_Pos(0.0f), DDDD_Pos_Pos_Mov_Pos(0.0f), DDDD_Mov_Pos_Mov_Pos(0.0f),
            DDDD_Prof_Pos_Mov_Pos(0.0f), DDDD_Pos_Mov_Mov_Pos(0.0f), DDDD_Mov_Mov_Mov_Pos(0.0f),
            DDDD_Prof_Mov_Mov_Pos(0.0f), DDDD_Pos_Prof_Mov_Pos(0.0f), DDDD_Mov_Prof_Mov_Pos(0.0f),
            DDDD_Prof_Prof_Mov_Pos(0.0f), DDDD_Pos_Pos_Prof_Pos(0.0f), DDDD_Mov_Pos_Prof_Pos(0.0f),
            DDDD_Prof_Pos_Prof_Pos(0.0f), DDDD_Pos_Mov_Prof_Pos(0.0f), DDDD_Mov_Mov_Prof_Pos(0.0f),
            DDDD_Prof_Mov_Prof_Pos(0.0f), DDDD_Pos_Prof_Prof_Pos(0.0f), DDDD_Mov_Prof_Prof_Pos(0.0f),
            DDDD_Prof_Prof_Prof_Pos(0.0f), DDDD_Pos_Pos_Mov_Mov(0.0f), DDDD_Mov_Pos_Mov_Mov(0.0f),
            DDDD_Prof_Pos_Mov_Mov(0.0f), DDDD_Pos_Mov_Mov_Mov(0.0f), DDDD_Mov_Mov_Mov_Mov(0.0f),
            DDDD_Prof_Mov_Mov_Mov(0.0f), DDDD_Pos_Prof_Mov_Mov(0.0f), DDDD_Mov_Prof_Mov_Mov(0.0f),
            DDDD_Prof_Prof_Mov_Mov(0.0f), DDDD_Pos_Pos_Pos_Mov(0.0f), DDDD_Mov_Pos_Pos_Mov(0.0f),
            DDDD_Prof_Pos_Pos_Mov(0.0f), DDDD_Pos_Mov_Pos_Mov(0.0f), DDDD_Mov_Mov_Pos_Mov(0.0f),
            DDDD_Prof_Mov_Pos_Mov(0.0f), DDDD_Pos_Prof_Pos_Mov(0.0f), DDDD_Mov_Prof_Pos_Mov(0.0f),
            DDDD_Prof_Prof_Pos_Mov(0.0f), DDDD_Pos_Pos_Prof_Mov(0.0f), DDDD_Mov_Pos_Prof_Mov(0.0f),
            DDDD_Prof_Pos_Prof_Mov(0.0f), DDDD_Pos_Mov_Prof_Mov(0.0f), DDDD_Mov_Mov_Prof_Mov(0.0f),
            DDDD_Prof_Mov_Prof_Mov(0.0f), DDDD_Pos_Prof_Prof_Mov(0.0f), DDDD_Mov_Prof_Prof_Mov(0.0f),
            DDDD_Prof_Prof_Prof_Mov(0.0f), DDDD_Pos_Pos_Prof_Prof(0.0f), DDDD_Mov_Pos_Prof_Prof(0.0f),
            DDDD_Prof_Pos_Prof_Prof(0.0f), DDDD_Pos_Mov_Prof_Prof(0.0f), DDDD_Mov_Mov_Prof_Prof(0.0f),
            DDDD_Prof_Mov_Prof_Prof(0.0f), DDDD_Pos_Prof_Prof_Prof(0.0f), DDDD_Mov_Prof_Prof_Prof(0.0f),
            DDDD_Prof_Prof_Prof_Prof(0.0f), DDDD_Pos_Pos_Pos_Prof(0.0f), DDDD_Mov_Pos_Pos_Prof(0.0f),
            DDDD_Prof_Pos_Pos_Prof(0.0f), DDDD_Pos_Mov_Pos_Prof(0.0f), DDDD_Mov_Mov_Pos_Prof(0.0f),
            DDDD_Prof_Mov_Pos_Prof(0.0f), DDDD_Pos_Prof_Pos_Prof(0.0f), DDDD_Mov_Prof_Pos_Prof(0.0f),
            DDDD_Prof_Prof_Pos_Prof(0.0f), DDDD_Pos_Pos_Mov_Prof(0.0f), DDDD_Mov_Pos_Mov_Prof(0.0f),
            DDDD_Prof_Pos_Mov_Prof(0.0f), DDDD_Pos_Mov_Mov_Prof(0.0f), DDDD_Mov_Mov_Mov_Prof(0.0f),
            DDDD_Prof_Mov_Mov_Prof(0.0f), DDDD_Pos_Prof_Mov_Prof(0.0f), DDDD_Mov_Prof_Mov_Prof(0.0f),
            DDDD_Prof_Prof_Mov_Prof(0.0f), taylor_sensitivity_1(0.0f), taylor_sensitivity_2(0.0f),
            taylor_sensitivity_3(0.0f) {}

    void Fill_Parameters(const Simulation_Results &sim) {
        pnl = sim.pnl;
        drawdown_count = sim.drawdown_count;
    }
};

struct Yesterday_Params {
    int strategy_variant;
    float pos_param, mov_param, prof_param;
    bool empty_param;

    Yesterday_Params() : strategy_variant(-1), pos_param(0.0f), mov_param(0.0f), prof_param(0.0f), empty_param(true) {}

    bool operator<(const Yesterday_Params &rhs) const { return strategy_variant < rhs.strategy_variant; }

    void fill(const int &s_variant, const float &p, const float &m, const float &pr) {
        strategy_variant = s_variant;
        pos_param = p;
        mov_param = m;
        prof_param = pr;
        empty_param = false;
    }
};

std::string get_next_date(const std::string &date_today);

time_t String_To_Time_T(const std::string &date);

std::string Time_T_To_String(const time_t date);

void Sim_To_CSV(const std::vector<Simulation_Results> &, const std::string &, const std::string &);

#endif // !UTILITIES_H
