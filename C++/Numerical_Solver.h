//
// Created by Praneet Shaw on 3/20/23.
//

#ifndef TEST_NUMERICAL_SOLVER_H
#define TEST_NUMERICAL_SOLVER_H

#include<vector>
#include<iostream>
#include<cmath>
#include<tuple>
#include<string>
#include<map>
#include<algorithm>
#include "Utilities.h"
#include "Drawdown_Sorting_Pairs.h"
#include "Pnl_Sorting_Pairs.h"
#include "TS1_Pairs.h"
#include "TS2_Pairs.h"
#include "TS3_Pairs.h"
#include "PDev_Pairs.h"
#include "MDev_Pairs.h"
#include "PRDev_Pairs.h"
#include "PDiff_Pairs.h"
#include "MDiff_Pairs.h"
#include "PRDiff_Pairs.h"

using std::vector;
using std::map;
using std::tuple;
using std::string;

class Numerical_Solver {

private:
    const Config *config;
    short int pos_length, mov_length, prof_length;
    vector<Numerical_Point *> lattice_points;
    vector<vector<vector<Numerical_Point *>>> pos_mov_prof_cube, mov_prof_pos_cube, prof_pos_mov_cube;
    vector<float *> pos_dev, mov_dev, prof_dev;
    Yesterday_Params y_params;
    int csv_counter;

    float Descriptive_Statistics(const vector<vector<Numerical_Point *>> &) const;

    void Mean_Variance();

    void First_Order_Derivative();

    void Second_Order_Derivative();

    void Third_Order_Derivative();

    void Fourth_Order_Derivative();

    float Taylor_Expansion(int, int, int, const Numerical_Point *) const;

    void Taylor_Sensitivity();

    void Nearness_Factor();

public:
    Numerical_Solver(const Config &, const int &, const vector<float> &, const vector<float> &, const vector<float> &);

    ~Numerical_Solver();

    Numerical_Solver(const Numerical_Solver &other) = default;

    Numerical_Solver &operator=(const Numerical_Solver &other) = default;

    void print(int, int) const;

    void Infuse_Simulation_Results(const vector<Simulation_Results> &);

    [[nodiscard]] Numerical_Sim_Results Optimize();

    void Update_Yesterday_Params(const Yesterday_Params &);

    void Update_Expiry_Params(const Yesterday_Params &);

    Yesterday_Params Get_Yesterday_Params() const;

    void Update_Metadata(Numerical_Sim_Results &) const;

    void To_Csv() const;
};


#endif //TEST_NUMERICAL_SOLVER_H
