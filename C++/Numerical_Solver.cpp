//
// Created by Praneet Shaw on 3/20/23.
//
#ifndef TEST_NUMERICAL_SOLVER_CPP
#define TEST_NUMERICAL_SOLVER_CPP

#include "Numerical_Solver.h"


Numerical_Solver::Numerical_Solver(const Config &conf, const int &strat_variant, const vector<float> &pos_params,
                                   const vector<float> &mov_params, const vector<float> &prof_params) {

    csv_counter = 0;

    config = &conf;
    y_params = Yesterday_Params();
    y_params.strategy_variant = strat_variant;
    pos_length = pos_params.size();
    mov_length = mov_params.size();
    prof_length = prof_params.size();

    //heap memory for point parameters
    pos_dev.reserve(pos_length);
    mov_dev.reserve(mov_length);
    prof_dev.reserve(prof_length);

    //creating internal params for lattice points
    int n = pos_length > mov_length ? (pos_length > prof_length ? pos_length : prof_length) :
            (mov_length > prof_length ? mov_length : prof_length);

    for (int i = 0; i < n; i++) {
        if (i < pos_length) { pos_dev.push_back(new float(0.0f)); }
        if (i < mov_length) { mov_dev.push_back(new float(0.0f)); }
        if (i < prof_length) { prof_dev.push_back(new float(0.0f)); }
    }

    //Creating Points
    lattice_points.reserve(pos_length * mov_length * prof_length);
    for (auto &pos: pos_params) {
        for (auto &mov: mov_params) {
            for (auto &prof: prof_params) {
                lattice_points.push_back(new Numerical_Point(pos, mov, prof));
            }
        }
    }

    //Creating Lattices
    int temp_position = 0;
    pos_mov_prof_cube.reserve(pos_length);
    for (int i = 0; i < pos_length; i++) {
        pos_mov_prof_cube.push_back(vector<vector<Numerical_Point *>>());
        pos_mov_prof_cube.back().reserve(mov_length);
        for (int j = 0; j < mov_length; j++) {
            pos_mov_prof_cube.back().push_back(vector<Numerical_Point *>());
            pos_mov_prof_cube.back().back().reserve(prof_length);
            for (int k = 0; k < prof_length; k++) {
                temp_position = i * (mov_length * prof_length) + j * (prof_length) + k;
                pos_mov_prof_cube.back().back().push_back(lattice_points[temp_position]);
                pos_mov_prof_cube.back().back().back()->pos_dev = pos_dev[i];
            }
        }
    }

    mov_prof_pos_cube.reserve(mov_length);
    for (int i = 0; i < mov_length; i++) {
        mov_prof_pos_cube.push_back(vector<vector<Numerical_Point *>>());
        mov_prof_pos_cube.back().reserve(prof_length);
        for (int j = 0; j < prof_length; j++) {
            mov_prof_pos_cube.back().push_back(vector<Numerical_Point *>());
            mov_prof_pos_cube.back().back().reserve(pos_length);
            for (int k = 0; k < pos_length; k++) {
                temp_position = k * (mov_length * prof_length) + i * (prof_length) + j;
                mov_prof_pos_cube.back().back().push_back(lattice_points[temp_position]);
                mov_prof_pos_cube.back().back().back()->mov_dev = mov_dev[i];
            }
        }
    }

    prof_pos_mov_cube.reserve(prof_length);
    for (int i = 0; i < prof_length; i++) {
        prof_pos_mov_cube.push_back(vector<vector<Numerical_Point *>>());
        prof_pos_mov_cube.back().reserve(pos_length);
        for (int j = 0; j < pos_length; j++) {
            prof_pos_mov_cube.back().push_back(vector<Numerical_Point *>());
            prof_pos_mov_cube.back().back().reserve(mov_length);
            for (int k = 0; k < mov_length; k++) {
                temp_position = j * (mov_length * prof_length) + k * (prof_length) + i;
                prof_pos_mov_cube.back().back().push_back(lattice_points[temp_position]);
                prof_pos_mov_cube.back().back().back()->prof_dev = prof_dev[i];
            }
        }
    }
}

Numerical_Solver::~Numerical_Solver() {

    for (auto &point: lattice_points) { delete point; }
    lattice_points.clear();
    pos_mov_prof_cube.clear();
    mov_prof_pos_cube.clear();
    prof_pos_mov_cube.clear();

    int n = pos_length > mov_length ? (pos_length > prof_length ? pos_length : prof_length) :
            (mov_length > prof_length ? mov_length : prof_length);

    for (int i = 0; i < n; i++) {
        if (i < pos_length) { delete pos_dev[i]; }
        if (i < mov_length) { delete mov_dev[i]; }
        if (i < prof_length) { delete prof_dev[i]; }
    }

}

void Numerical_Solver::print(int m, int n) const {
    for (int i = m; i < n; i++) {
        std::cout << std::endl << std::endl;
        for (int j = m; j < n; j++) {
            std::cout << std::endl;
            for (int k = m; k < n; k++) {
                std::cout << "(" << pos_mov_prof_cube[i][j][k]->pos_param << "," <<
                          pos_mov_prof_cube[i][j][k]->mov_param << "," << pos_mov_prof_cube[i][j][k]->prof_param
                          << ", Pos_Dev: " << *pos_mov_prof_cube[i][j][k]->pos_dev << ", Mov_Dev: " <<
                          *pos_mov_prof_cube[i][j][k]->mov_dev << ", Prof_Dev: "
                          << *pos_mov_prof_cube[i][j][k]->prof_dev
                          << ", PnL: " << pos_mov_prof_cube[i][j][k]->pnl << ", Taylor_Sensitivity_1: "
                          << pos_mov_prof_cube[i][j][k]->taylor_sensitivity_1 << ", Taylor_Sensitivity_2: " <<
                          pos_mov_prof_cube[i][j][k]->taylor_sensitivity_2 << ", Taylor_Sensitivity_3: " <<
                          pos_mov_prof_cube[i][j][k]->taylor_sensitivity_3 << std::endl;
                /*
                std::cout << "(" << pos_mov_prof_cube[i][j][k]->pos_param << "," <<
                          pos_mov_prof_cube[i][j][k]->mov_param << "," << pos_mov_prof_cube[i][j][k]->prof_param
                          << ") -> PnL: "<<pos_mov_prof_cube[i][j][k]->pnl<<", LHD_Pos: "<<pos_mov_prof_cube[i][j][k]->LHD_Pos<<
                          ", RHD_Pos: "<<pos_mov_prof_cube[i][j][k]->RHD_Pos<<", D_Pos: "<<pos_mov_prof_cube[i][j][k]->D_Pos<<
                          ", LHD_Mov: "<<pos_mov_prof_cube[i][j][k]->LHD_Mov<<", RHD_Mov: "<<pos_mov_prof_cube[i][j][k]->RHD_Mov<<
                          ", D_Mov: "<<pos_mov_prof_cube[i][j][k]->D_Mov<<", LHD_Prof: "<<pos_mov_prof_cube[i][j][k]->LHD_Prof<<
                          ", RHD_Prof: "<<pos_mov_prof_cube[i][j][k]->RHD_Prof<<", D_Prof: "<<pos_mov_prof_cube[i][j][k]->D_Prof<<std::endl;*/
            }
        }
    }
}

float Numerical_Solver::Descriptive_Statistics(const vector<vector<Numerical_Point *>> &Frame) const {
    float mean = 0.0f, stdev = 0.0f, counter = 0.0f;

    for (auto &rows: Frame) {
        for (auto &cols: rows) {
            mean += cols->pnl;
            stdev += std::powf(cols->pnl, 2);
            counter++;
        }
    }

    mean /= counter;
    stdev = sqrtf(std::max((stdev / counter) - std::powf(mean, 2),1.0f));

    if (abs(stdev - 0.0f) < 0.00001f) { return mean / 7500.0f; }
    else { return mean / stdev; }
}

void Numerical_Solver::Mean_Variance() {
    int counter = 0;
    for (auto &pos_frame: pos_mov_prof_cube) {
        *pos_dev[counter] = Descriptive_Statistics(pos_frame);
        counter++;
    }

    counter = 0;
    for (auto &mov_frame: mov_prof_pos_cube) {
        *mov_dev[counter] = Descriptive_Statistics(mov_frame);
        counter++;
    }

    counter = 0;
    for (auto &prof_frame: prof_pos_mov_cube) {
        *prof_dev[counter] = Descriptive_Statistics(prof_frame);
        counter++;
    }
}

void Numerical_Solver::Infuse_Simulation_Results(const vector<Simulation_Results> &Simulation_Results) {

    int counter = 0;
    for (auto &sim: Simulation_Results) {
        lattice_points[counter]->Fill_Parameters(sim);
        counter++;
    }

    Mean_Variance();
    First_Order_Derivative();
    Second_Order_Derivative();
    Third_Order_Derivative();
    Fourth_Order_Derivative();
    Taylor_Sensitivity();
    Nearness_Factor();
    //To_Csv();
    //csv_counter++;
}

void Numerical_Solver::First_Order_Derivative() {
    for (int pos_counter = 0; pos_counter < pos_length; pos_counter++) {
        for (int mov_counter = 0; mov_counter < mov_length; mov_counter++) {
            for (int prof_counter = 0; prof_counter < prof_length; prof_counter++) {
                if (pos_counter != pos_length - 1) {
                    pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Pos =
                            (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->pnl -
                             pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pnl) / config->pos_increment;
                    pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->LHD_Pos =
                            pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Pos;

                    if (pos_counter >= 1) {
                        pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->D_Pos =
                                (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Pos +
                                 pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->LHD_Pos) / 2.0f;
                    }
                }
                if (mov_counter != mov_length - 1) {
                    pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Mov =
                            (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->pnl -
                             pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pnl) / config->mov_increment;
                    pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->LHD_Mov =
                            pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Mov;

                    if (mov_counter >= 1) {
                        pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->D_Mov =
                                (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Mov +
                                 pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->LHD_Mov) / 2.0f;
                    }
                }
                if (prof_counter != prof_length - 1) {
                    pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Prof =
                            (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->pnl -
                             pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pnl) / config->prof_increment;
                    pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->LHD_Prof =
                            pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Prof;

                    if (prof_counter >= 1) {
                        pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->D_Prof =
                                (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Prof +
                                 pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->LHD_Prof) / 2.0f;
                    }
                }
            }
        }
    }
}

void Numerical_Solver::Second_Order_Derivative() {
    for (int pos_counter = 1; pos_counter < pos_length - 1; pos_counter++) {
        for (int mov_counter = 1; mov_counter < mov_length - 1; mov_counter++) {
            for (int prof_counter = 1; prof_counter < prof_length - 1; prof_counter++) {
                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DD_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->LHD_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DD_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->D_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->D_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DD_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->D_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->D_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DD_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->LHD_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DD_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->D_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->D_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DD_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->D_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->D_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DD_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->RHD_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->LHD_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DD_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->D_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->D_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DD_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->D_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->D_Prof)
                        / (2.0f * config->mov_increment);
            }
        }
    }
}

void Numerical_Solver::Third_Order_Derivative() {
    for (int pos_counter = 2; pos_counter < pos_length - 2; pos_counter++) {
        for (int mov_counter = 2; mov_counter < mov_length - 2; mov_counter++) {
            for (int prof_counter = 2; prof_counter < prof_length - 2; prof_counter++) {
                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Pos_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DD_Pos_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DD_Pos_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Mov_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DD_Pos_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DD_Pos_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Prof_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DD_Pos_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DD_Pos_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Pos_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DD_Mov_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DD_Mov_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Mov_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DD_Mov_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DD_Mov_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Prof_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DD_Mov_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DD_Mov_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Pos_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DD_Prof_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DD_Prof_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Mov_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DD_Prof_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DD_Prof_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Prof_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DD_Prof_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DD_Prof_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Pos_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DD_Mov_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DD_Mov_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Mov_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DD_Mov_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DD_Mov_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Prof_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DD_Mov_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DD_Mov_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Pos_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DD_Pos_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DD_Pos_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Mov_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DD_Pos_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DD_Pos_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Prof_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DD_Pos_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DD_Pos_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Pos_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DD_Prof_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DD_Prof_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Mov_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DD_Prof_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DD_Prof_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Prof_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DD_Prof_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DD_Prof_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Pos_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DD_Prof_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DD_Prof_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Mov_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DD_Prof_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DD_Prof_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Prof_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DD_Prof_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DD_Prof_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Pos_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DD_Pos_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DD_Pos_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Mov_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DD_Pos_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DD_Pos_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Prof_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DD_Pos_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DD_Pos_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Pos_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DD_Mov_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DD_Mov_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Mov_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DD_Mov_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DD_Mov_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDD_Prof_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DD_Mov_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DD_Mov_Prof)
                        / (2.0f * config->prof_increment);
            }
        }
    }
}

void Numerical_Solver::Fourth_Order_Derivative() {
    for (int pos_counter = 3; pos_counter < pos_length - 3; pos_counter++) {
        for (int mov_counter = 3; mov_counter < mov_length - 3; mov_counter++) {
            for (int prof_counter = 3; prof_counter < prof_length - 3; prof_counter++) {
                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Pos_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Pos_Pos_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Pos_Pos_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Pos_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Pos_Pos_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Pos_Pos_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Pos_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Pos_Pos_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Pos_Pos_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Mov_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Mov_Pos_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Mov_Pos_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Mov_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Mov_Pos_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Mov_Pos_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Mov_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Mov_Pos_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Mov_Pos_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Prof_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Prof_Pos_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Prof_Pos_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Prof_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Prof_Pos_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Prof_Pos_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Prof_Pos_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Prof_Pos_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Prof_Pos_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Pos_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Pos_Mov_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Pos_Mov_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Pos_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Pos_Mov_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Pos_Mov_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Pos_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Pos_Mov_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Pos_Mov_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Mov_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Mov_Mov_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Mov_Mov_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Mov_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Mov_Mov_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Mov_Mov_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Mov_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Mov_Mov_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Mov_Mov_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Prof_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Prof_Mov_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Prof_Mov_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Prof_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Prof_Mov_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Prof_Mov_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Prof_Mov_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Prof_Mov_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Prof_Mov_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Pos_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Pos_Prof_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Pos_Prof_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Pos_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Pos_Prof_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Pos_Prof_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Pos_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Pos_Prof_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Pos_Prof_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Mov_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Mov_Prof_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Mov_Prof_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Mov_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Mov_Prof_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Mov_Prof_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Mov_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Mov_Prof_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Mov_Prof_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Prof_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Prof_Prof_Pos -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Prof_Prof_Pos)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Prof_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Prof_Prof_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Prof_Prof_Pos)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Prof_Prof_Pos =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Prof_Prof_Pos -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Prof_Prof_Pos)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Pos_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Pos_Mov_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Pos_Mov_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Pos_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Pos_Mov_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Pos_Mov_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Pos_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Pos_Mov_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Pos_Mov_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Mov_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Mov_Mov_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Mov_Mov_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Mov_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Mov_Mov_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Mov_Mov_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Mov_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Mov_Mov_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Mov_Mov_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Prof_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Prof_Mov_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Prof_Mov_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Prof_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Prof_Mov_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Prof_Mov_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Prof_Mov_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Prof_Mov_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Prof_Mov_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Pos_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Pos_Pos_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Pos_Pos_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Pos_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Pos_Pos_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Pos_Pos_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Pos_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Pos_Pos_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Pos_Pos_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Mov_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Mov_Pos_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Mov_Pos_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Mov_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Mov_Pos_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Mov_Pos_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Mov_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Mov_Pos_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Mov_Pos_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Prof_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Prof_Pos_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Prof_Pos_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Prof_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Prof_Pos_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Prof_Pos_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Prof_Pos_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Prof_Pos_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Prof_Pos_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Pos_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Pos_Prof_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Pos_Prof_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Pos_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Pos_Prof_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Pos_Prof_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Pos_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Pos_Prof_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Pos_Prof_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Mov_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Mov_Prof_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Mov_Prof_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Mov_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Mov_Prof_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Mov_Prof_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Mov_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Mov_Prof_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Mov_Prof_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Prof_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Prof_Prof_Mov -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Prof_Prof_Mov)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Prof_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Prof_Prof_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Prof_Prof_Mov)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Prof_Prof_Mov =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Prof_Prof_Mov -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Prof_Prof_Mov)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Pos_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Pos_Prof_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Pos_Prof_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Pos_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Pos_Prof_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Pos_Prof_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Pos_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Pos_Prof_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Pos_Prof_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Mov_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Mov_Prof_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Mov_Prof_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Mov_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Mov_Prof_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Mov_Prof_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Mov_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Mov_Prof_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Mov_Prof_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Prof_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Prof_Prof_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Prof_Prof_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Prof_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Prof_Prof_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Prof_Prof_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Prof_Prof_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Prof_Prof_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Prof_Prof_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Pos_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Pos_Pos_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Pos_Pos_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Pos_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Pos_Pos_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Pos_Pos_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Pos_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Pos_Pos_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Pos_Pos_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Mov_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Mov_Pos_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Mov_Pos_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Mov_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Mov_Pos_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Mov_Pos_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Mov_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Mov_Pos_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Mov_Pos_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Prof_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Prof_Pos_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Prof_Pos_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Prof_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Prof_Pos_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Prof_Pos_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Prof_Pos_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Prof_Pos_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Prof_Pos_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Pos_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Pos_Mov_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Pos_Mov_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Pos_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Pos_Mov_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Pos_Mov_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Pos_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Pos_Mov_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Pos_Mov_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Mov_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Mov_Mov_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Mov_Mov_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Mov_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Mov_Mov_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Mov_Mov_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Mov_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Mov_Mov_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Mov_Mov_Prof)
                        / (2.0f * config->prof_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Pos_Prof_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter + 1][mov_counter][prof_counter]->DDD_Prof_Mov_Prof -
                         pos_mov_prof_cube[pos_counter - 1][mov_counter][prof_counter]->DDD_Prof_Mov_Prof)
                        / (2.0f * config->pos_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Mov_Prof_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter + 1][prof_counter]->DDD_Prof_Mov_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter - 1][prof_counter]->DDD_Prof_Mov_Prof)
                        / (2.0f * config->mov_increment);

                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->DDDD_Prof_Prof_Mov_Prof =
                        (pos_mov_prof_cube[pos_counter][mov_counter][prof_counter + 1]->DDD_Prof_Mov_Prof -
                         pos_mov_prof_cube[pos_counter][mov_counter][prof_counter - 1]->DDD_Prof_Mov_Prof)
                        / (2.0f * config->prof_increment);
            }
        }
    }
}

float Numerical_Solver::Taylor_Expansion(int p_factor, int m_factor, int pr_factor, const Numerical_Point *Num) const {
    float first_order, second_order, third_order, fourth_order;
    float pos_increment = config->pos_increment * p_factor;
    float mov_increment = config->mov_increment * m_factor;
    float prof_increment = config->prof_increment * pr_factor;

    first_order = Num->D_Pos * pos_increment + Num->D_Mov * mov_increment + Num->D_Prof * prof_increment;

    second_order =
            (Num->DD_Pos_Pos * (pos_increment * pos_increment) + Num->DD_Mov_Pos * (mov_increment * pos_increment) +
             Num->DD_Prof_Pos * (prof_increment * pos_increment) + Num->DD_Mov_Mov * (mov_increment * mov_increment) +
             Num->DD_Pos_Mov * (pos_increment * mov_increment) + Num->DD_Prof_Mov * (prof_increment * mov_increment) +
             Num->DD_Prof_Prof * (prof_increment * prof_increment) +
             Num->DD_Pos_Prof * (pos_increment * prof_increment) +
             Num->DD_Mov_Prof * (mov_increment * prof_increment)) / 2.0f;

    third_order = (Num->DDD_Pos_Pos_Pos * (pos_increment * pos_increment * pos_increment) +
                   Num->DDD_Mov_Pos_Pos * (mov_increment *
                                           pos_increment * pos_increment) +
                   Num->DDD_Prof_Pos_Pos * (prof_increment * pos_increment * pos_increment) +
                   Num->DDD_Pos_Mov_Pos * (pos_increment * mov_increment * pos_increment) +
                   Num->DDD_Mov_Mov_Pos * (mov_increment *
                                           mov_increment * pos_increment) +
                   Num->DDD_Prof_Mov_Pos * (prof_increment * mov_increment * pos_increment) +
                   Num->DDD_Pos_Prof_Pos * (pos_increment * prof_increment * pos_increment) +
                   Num->DDD_Mov_Prof_Pos * (mov_increment *
                                            prof_increment * pos_increment) +
                   Num->DDD_Prof_Prof_Pos * (prof_increment * prof_increment * pos_increment) +
                   Num->DDD_Pos_Mov_Mov * (pos_increment * mov_increment * mov_increment) +
                   Num->DDD_Mov_Mov_Mov * (mov_increment *
                                           mov_increment * mov_increment) +
                   Num->DDD_Prof_Mov_Mov * (prof_increment * mov_increment * mov_increment) +
                   Num->DDD_Pos_Pos_Mov * (pos_increment * pos_increment * mov_increment) +
                   Num->DDD_Mov_Pos_Mov * (mov_increment *
                                           pos_increment * mov_increment) +
                   Num->DDD_Prof_Pos_Mov * (prof_increment * pos_increment * mov_increment) +
                   Num->DDD_Pos_Prof_Mov * (pos_increment * prof_increment * mov_increment) +
                   Num->DDD_Mov_Prof_Mov * (mov_increment *
                                            prof_increment * mov_increment) +
                   Num->DDD_Prof_Prof_Mov * (prof_increment * prof_increment * mov_increment) +
                   Num->DDD_Pos_Prof_Prof * (pos_increment * prof_increment * prof_increment) +
                   Num->DDD_Mov_Prof_Prof * (mov_increment *
                                             prof_increment * prof_increment) +
                   Num->DDD_Prof_Prof_Prof * (prof_increment * prof_increment * prof_increment) +
                   Num->DDD_Pos_Pos_Prof * (pos_increment * pos_increment * prof_increment) +
                   Num->DDD_Mov_Pos_Prof * (mov_increment *
                                            pos_increment * prof_increment) +
                   Num->DDD_Prof_Pos_Prof * (prof_increment * pos_increment * prof_increment) +
                   Num->DDD_Pos_Mov_Prof * (pos_increment * mov_increment * prof_increment) +
                   Num->DDD_Mov_Mov_Prof * (mov_increment *
                                            mov_increment * prof_increment) +
                   Num->DDD_Prof_Mov_Prof * (prof_increment * mov_increment * prof_increment)) / 6.0f;

    fourth_order = (Num->DDDD_Pos_Pos_Pos_Pos * (pos_increment * pos_increment * pos_increment * pos_increment) +
                    Num->DDDD_Mov_Pos_Pos_Pos * (mov_increment * pos_increment * pos_increment * pos_increment) +
                    Num->DDDD_Prof_Pos_Pos_Pos * (prof_increment * pos_increment * pos_increment * pos_increment) +
                    Num->DDDD_Pos_Mov_Pos_Pos * (pos_increment * mov_increment * pos_increment * pos_increment) +
                    Num->DDDD_Mov_Mov_Pos_Pos * (mov_increment * mov_increment * pos_increment * pos_increment) +
                    Num->DDDD_Prof_Mov_Pos_Pos * (prof_increment * mov_increment * pos_increment * pos_increment) +
                    Num->DDDD_Pos_Prof_Pos_Pos * (pos_increment * prof_increment * pos_increment * pos_increment) +
                    Num->DDDD_Mov_Prof_Pos_Pos * (mov_increment * prof_increment * pos_increment * pos_increment) +
                    Num->DDDD_Prof_Prof_Pos_Pos * (prof_increment * prof_increment * pos_increment * pos_increment) +
                    Num->DDDD_Pos_Pos_Mov_Pos * (pos_increment * pos_increment * mov_increment * pos_increment) +
                    Num->DDDD_Mov_Pos_Mov_Pos * (mov_increment * pos_increment * mov_increment * pos_increment) +
                    Num->DDDD_Prof_Pos_Mov_Pos * (prof_increment * pos_increment * mov_increment * pos_increment) +
                    Num->DDDD_Pos_Mov_Mov_Pos * (pos_increment * mov_increment * mov_increment * pos_increment) +
                    Num->DDDD_Mov_Mov_Mov_Pos * (mov_increment * mov_increment * mov_increment * pos_increment) +
                    Num->DDDD_Prof_Mov_Mov_Pos * (prof_increment * mov_increment * mov_increment * pos_increment) +
                    Num->DDDD_Pos_Prof_Mov_Pos * (pos_increment * prof_increment * mov_increment * pos_increment) +
                    Num->DDDD_Mov_Prof_Mov_Pos * (mov_increment * prof_increment * mov_increment * pos_increment) +
                    Num->DDDD_Prof_Prof_Mov_Pos * (prof_increment * prof_increment * mov_increment * pos_increment) +
                    Num->DDDD_Pos_Pos_Prof_Pos * (pos_increment * pos_increment * prof_increment * pos_increment) +
                    Num->DDDD_Mov_Pos_Prof_Pos * (mov_increment * pos_increment * prof_increment * pos_increment) +
                    Num->DDDD_Prof_Pos_Prof_Pos * (prof_increment * pos_increment * prof_increment * pos_increment) +
                    Num->DDDD_Pos_Mov_Prof_Pos * (pos_increment * mov_increment * prof_increment * pos_increment) +
                    Num->DDDD_Mov_Mov_Prof_Pos * (mov_increment * mov_increment * prof_increment * pos_increment) +
                    Num->DDDD_Prof_Mov_Prof_Pos * (prof_increment * mov_increment * prof_increment * pos_increment) +
                    Num->DDDD_Pos_Prof_Prof_Pos * (pos_increment * prof_increment * prof_increment * pos_increment) +
                    Num->DDDD_Mov_Prof_Prof_Pos * (mov_increment * prof_increment * prof_increment * pos_increment) +
                    Num->DDDD_Prof_Prof_Prof_Pos * (prof_increment * prof_increment * prof_increment * pos_increment) +
                    Num->DDDD_Pos_Pos_Mov_Mov * (pos_increment * pos_increment * mov_increment * mov_increment) +
                    Num->DDDD_Mov_Pos_Mov_Mov * (mov_increment * pos_increment * mov_increment * mov_increment) +
                    Num->DDDD_Prof_Pos_Mov_Mov * (prof_increment * pos_increment * mov_increment * mov_increment) +
                    Num->DDDD_Pos_Mov_Mov_Mov * (pos_increment * mov_increment * mov_increment * mov_increment) +
                    Num->DDDD_Mov_Mov_Mov_Mov * (mov_increment * mov_increment * mov_increment * mov_increment) +
                    Num->DDDD_Prof_Mov_Mov_Mov * (prof_increment * mov_increment * mov_increment * mov_increment) +
                    Num->DDDD_Pos_Prof_Mov_Mov * (pos_increment * prof_increment * mov_increment * mov_increment) +
                    Num->DDDD_Mov_Prof_Mov_Mov * (mov_increment * prof_increment * mov_increment * mov_increment) +
                    Num->DDDD_Prof_Prof_Mov_Mov * (prof_increment * prof_increment * mov_increment * mov_increment) +
                    Num->DDDD_Pos_Pos_Pos_Mov * (pos_increment * pos_increment * pos_increment * mov_increment) +
                    Num->DDDD_Mov_Pos_Pos_Mov * (mov_increment * pos_increment * pos_increment * mov_increment) +
                    Num->DDDD_Prof_Pos_Pos_Mov * (prof_increment * pos_increment * pos_increment * mov_increment) +
                    Num->DDDD_Pos_Mov_Pos_Mov * (pos_increment * mov_increment * pos_increment * mov_increment) +
                    Num->DDDD_Mov_Mov_Pos_Mov * (mov_increment * mov_increment * pos_increment * mov_increment) +
                    Num->DDDD_Prof_Mov_Pos_Mov * (prof_increment * mov_increment * pos_increment * mov_increment) +
                    Num->DDDD_Pos_Prof_Pos_Mov * (pos_increment * prof_increment * pos_increment * mov_increment) +
                    Num->DDDD_Mov_Prof_Pos_Mov * (mov_increment * prof_increment * pos_increment * mov_increment) +
                    Num->DDDD_Prof_Prof_Pos_Mov * (prof_increment * prof_increment * pos_increment * mov_increment) +
                    Num->DDDD_Pos_Pos_Prof_Mov * (pos_increment * pos_increment * prof_increment * mov_increment) +
                    Num->DDDD_Mov_Pos_Prof_Mov * (mov_increment * pos_increment * prof_increment * mov_increment) +
                    Num->DDDD_Prof_Pos_Prof_Mov * (prof_increment * pos_increment * prof_increment * mov_increment) +
                    Num->DDDD_Pos_Mov_Prof_Mov * (pos_increment * mov_increment * prof_increment * mov_increment) +
                    Num->DDDD_Mov_Mov_Prof_Mov * (mov_increment * mov_increment * prof_increment * mov_increment) +
                    Num->DDDD_Prof_Mov_Prof_Mov * (prof_increment * mov_increment * prof_increment * mov_increment) +
                    Num->DDDD_Pos_Prof_Prof_Mov * (pos_increment * prof_increment * prof_increment * mov_increment) +
                    Num->DDDD_Mov_Prof_Prof_Mov * (mov_increment * prof_increment * prof_increment * mov_increment) +
                    Num->DDDD_Prof_Prof_Prof_Mov * (prof_increment * prof_increment * prof_increment * mov_increment) +
                    Num->DDDD_Pos_Pos_Prof_Prof * (pos_increment * pos_increment * prof_increment * prof_increment) +
                    Num->DDDD_Mov_Pos_Prof_Prof * (mov_increment * pos_increment * prof_increment * prof_increment) +
                    Num->DDDD_Prof_Pos_Prof_Prof * (prof_increment * pos_increment * prof_increment * prof_increment) +
                    Num->DDDD_Pos_Mov_Prof_Prof * (pos_increment * mov_increment * prof_increment * prof_increment) +
                    Num->DDDD_Mov_Mov_Prof_Prof * (mov_increment * mov_increment * prof_increment * prof_increment) +
                    Num->DDDD_Prof_Mov_Prof_Prof * (prof_increment * mov_increment * prof_increment * prof_increment) +
                    Num->DDDD_Pos_Prof_Prof_Prof * (pos_increment * prof_increment * prof_increment * prof_increment) +
                    Num->DDDD_Mov_Prof_Prof_Prof * (mov_increment * prof_increment * prof_increment * prof_increment) +
                    Num->DDDD_Prof_Prof_Prof_Prof *
                    (prof_increment * prof_increment * prof_increment * prof_increment) +
                    Num->DDDD_Pos_Pos_Pos_Prof * (pos_increment * pos_increment * pos_increment * prof_increment) +
                    Num->DDDD_Mov_Pos_Pos_Prof * (mov_increment * pos_increment * pos_increment * prof_increment) +
                    Num->DDDD_Prof_Pos_Pos_Prof * (prof_increment * pos_increment * pos_increment * prof_increment) +
                    Num->DDDD_Pos_Mov_Pos_Prof * (pos_increment * mov_increment * pos_increment * prof_increment) +
                    Num->DDDD_Mov_Mov_Pos_Prof * (mov_increment * mov_increment * pos_increment * prof_increment) +
                    Num->DDDD_Prof_Mov_Pos_Prof * (prof_increment * mov_increment * pos_increment * prof_increment) +
                    Num->DDDD_Pos_Prof_Pos_Prof * (pos_increment * prof_increment * pos_increment * prof_increment) +
                    Num->DDDD_Mov_Prof_Pos_Prof * (mov_increment * prof_increment * pos_increment * prof_increment) +
                    Num->DDDD_Prof_Prof_Pos_Prof * (prof_increment * prof_increment * pos_increment * prof_increment) +
                    Num->DDDD_Pos_Pos_Mov_Prof * (pos_increment * pos_increment * mov_increment * prof_increment) +
                    Num->DDDD_Mov_Pos_Mov_Prof * (mov_increment * pos_increment * mov_increment * prof_increment) +
                    Num->DDDD_Prof_Pos_Mov_Prof * (prof_increment * pos_increment * mov_increment * prof_increment) +
                    Num->DDDD_Pos_Mov_Mov_Prof * (pos_increment * mov_increment * mov_increment * prof_increment) +
                    Num->DDDD_Mov_Mov_Mov_Prof * (mov_increment * mov_increment * mov_increment * prof_increment) +
                    Num->DDDD_Prof_Mov_Mov_Prof * (prof_increment * mov_increment * mov_increment * prof_increment) +
                    Num->DDDD_Pos_Prof_Mov_Prof * (pos_increment * prof_increment * mov_increment * prof_increment) +
                    Num->DDDD_Mov_Prof_Mov_Prof * (mov_increment * prof_increment * mov_increment * prof_increment) +
                    Num->DDDD_Prof_Prof_Mov_Prof * (prof_increment * prof_increment * mov_increment * prof_increment)) /
                   24.0f;

    return abs(first_order + second_order + third_order + fourth_order);
}

void Numerical_Solver::Taylor_Sensitivity() {
    float sensitivity;
    for (int pos_counter = 3; pos_counter < pos_length - 3; pos_counter++) {
        for (int mov_counter = 3; mov_counter < mov_length - 3; mov_counter++) {
            for (int prof_counter = 3; prof_counter < prof_length - 3; prof_counter++) {

                sensitivity = 0.0f;
                for (int i = -1; i <= 1; i += 2) {
                    for (int j = -1; j <= 1; j += 2) {
                        for (int k = -1; k <= 1; k += 2) {
                            sensitivity += Taylor_Expansion(i, j, k,
                                                            pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]);
                        }
                    }
                }
                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_1 = sensitivity / 8.0f;

                sensitivity = 0.0f;
                for (int i = -2; i <= 2; i += 4) {
                    for (int j = -2; j <= 2; j += 4) {
                        for (int k = -2; k <= 2; k += 4) {
                            sensitivity += Taylor_Expansion(i, j, k,
                                                            pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]);
                        }
                    }
                }
                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_2 = sensitivity / 8.0f;

                sensitivity = 0.0f;
                for (int i = -3; i <= 3; i += 6) {
                    for (int j = -3; j <= 3; j += 6) {
                        for (int k = -3; k <= 3; k += 6) {
                            sensitivity += Taylor_Expansion(i, j, k,
                                                            pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]);
                        }
                    }
                }
                pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_3 = sensitivity / 8.0f;

                if (abs(pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pnl - 0.0f) < 0.1f) {
                    pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_1 = 2.0f;
                    pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_2 = 4.0f;
                    pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_3 = 6.0f;
                } else {
                    pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_1 /=
                            pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pnl;
                    pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_2 /=
                            pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pnl;
                    pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_3 /=
                            pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pnl;
                }
            }
        }
    }
}

void Numerical_Solver::Nearness_Factor() {
    for (auto &point: lattice_points) {
        if (y_params.empty_param) {
            point->pos_diff = 0.0f;
            point->mov_diff = 0.0f;
            point->prof_diff = 0.0f;
            point->pmpr_diff = 0.0f;
        } else {
            point->pos_diff = abs(point->pos_param - y_params.pos_param);
            point->mov_diff = abs(point->mov_param - y_params.mov_param);
            point->prof_diff = abs(point->prof_param - y_params.prof_param);
            point->pmpr_diff = 0.25f * point->pos_diff + 0.5f * point->mov_diff + 0.25f * point->prof_diff;
        }
    }
}

void Numerical_Solver::To_Csv() const {
    std::ofstream myfile;
    myfile.open(std::to_string(csv_counter) + std::string(".csv"));
    myfile << "PnL,Drawdown_Count,TS1,TS2,TS3,PDev,MDev,PRDev,PDiff,MDiff,PRDiff,PMPRDiff\n";
    for (int pos_counter = 3; pos_counter < pos_length - 3; pos_counter++) {
        for (int mov_counter = 3; mov_counter < mov_length - 3; mov_counter++) {
            for (int prof_counter = 3; prof_counter < prof_length - 3; prof_counter++) {
                myfile << pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pnl << "," <<
                       pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->drawdown_count << "," <<
                       pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_1 << "," <<
                       pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_2 << "," <<
                       pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->taylor_sensitivity_3 << "," <<
                       *pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pos_dev << "," <<
                       *pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->mov_dev << "," <<
                       *pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->prof_dev << "," <<
                       pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pos_diff << "," <<
                       pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->mov_diff << "," <<
                       pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->prof_diff << "," <<
                       pos_mov_prof_cube[pos_counter][mov_counter][prof_counter]->pmpr_diff << "\n";
            }
        }
    }
    myfile.close();
}

Numerical_Sim_Results Numerical_Solver::Optimize() {

    vector<Drawdown_Sorting_Pairs> Drawdown;
    vector<Pnl_Sorting_Pairs> PNL;
    vector<TS1_Pairs> TS1;
    vector<TS2_Pairs> TS2;
    vector<TS3_Pairs> TS3;
    vector<PDev_Pairs> PDev;
    vector<MDev_Pairs> MDev;
    vector<PRDev_Pairs> PRDev;
    vector<PDiff_Pairs> PDiff;
    vector<MDiff_Pairs> MDiff;
    vector<PRDiff_Pairs> PRDiff;
    bool all_negative;

    Drawdown.reserve((pos_length - 6) * (mov_length - 6) * (prof_length - 6));
    int position = 0;
    for (int pos_counter = 3; pos_counter < pos_length - 3; pos_counter++) {
        for (int mov_counter = 3; mov_counter < mov_length - 3; mov_counter++) {
            for (int prof_counter = 3; prof_counter < prof_length - 3; prof_counter++) {
                position = pos_counter * (mov_length * prof_length) + mov_counter * (prof_length) + prof_counter;
                Drawdown.push_back(Drawdown_Sorting_Pairs(position, lattice_points[position]->drawdown_count,
                                                          lattice_points[position]->pnl,
                                                          lattice_points[position]->taylor_sensitivity_1,
                                                          lattice_points[position]->taylor_sensitivity_2,
                                                          lattice_points[position]->taylor_sensitivity_3,
                                                          *lattice_points[position]->pos_dev,
                                                          *lattice_points[position]->mov_dev,
                                                          *lattice_points[position]->prof_dev,
                                                          lattice_points[position]->pos_diff,
                                                          lattice_points[position]->mov_diff,
                                                          lattice_points[position]->prof_diff));
            }
        }
    }

    std::sort(Drawdown.begin(), Drawdown.end());
    auto draw = Drawdown.begin();
    if (draw->drawdown_count > 1) { draw += (Drawdown.size() / 2) + 1; }
    else {
        for (auto iter = Drawdown.begin(); iter != Drawdown.end(); iter++) {
            if (iter->drawdown_count > 1) {
                draw = iter;
                break;
            }
        }
    }
    if (draw == Drawdown.begin()) { draw = Drawdown.end(); }

    PNL.reserve(Drawdown.size());
    for (auto iter = Drawdown.begin(); iter != draw; iter++) { PNL.push_back(Pnl_Sorting_Pairs(*iter)); }
    Drawdown.clear();
    std::sort(PNL.begin(), PNL.end());
    auto pnl = PNL.begin();
    if (pnl->pnl < 0.1f) {
        pnl += (PNL.size() / 2) + 1;
        all_negative = true;
    } else {
        all_negative = false;
        for (auto iter = PNL.begin(); iter != PNL.end(); iter++) {
            if (iter->pnl < 0.1f) {
                pnl = iter;
                break;
            }
        }
    }
    if (pnl == PNL.begin()) { pnl = PNL.end(); }

    PDiff.reserve(PNL.size());
    for (auto iter = PNL.begin(); iter != pnl; iter++) { PDiff.push_back(PDiff_Pairs(*iter)); }
    PNL.clear();
    std::sort(PDiff.begin(), PDiff.end());
    float temp_dev = (PDiff.begin() + PDiff.size() / 6)->pos_diff + 0.001f;
    MDiff.reserve(PDiff.size());
    for (auto iter = PDiff.begin(); iter != PDiff.end(); iter++) {
        if (iter->pos_diff <= temp_dev) { MDiff.push_back(MDiff_Pairs(*iter)); }
        else { break; }
    }
    PDiff.clear();

    std::sort(MDiff.begin(), MDiff.end());
    temp_dev = (MDiff.begin() + MDiff.size() / 8)->mov_diff + 0.001f;
    PRDiff.reserve(MDiff.size());
    for (auto iter = MDiff.begin(); iter != MDiff.end(); iter++) {
        if (iter->mov_diff <= temp_dev) { PRDiff.push_back(PRDiff_Pairs(*iter)); }
        else { break; }
    }
    MDiff.clear();

    std::sort(PRDiff.begin(), PRDiff.end());
    temp_dev = (PRDiff.begin() + PRDiff.size() / 6)->prof_diff + 0.001f;
    TS3.reserve(PRDiff.size());
    for (auto iter = PRDiff.begin(); iter != PRDiff.end(); iter++) {
        if (iter->prof_diff <= temp_dev) { TS3.push_back(TS3_Pairs(*iter)); }
        else { break; }
    }
    PRDiff.clear();

    if (!all_negative) {

        std::sort(TS3.begin(), TS3.end());
        temp_dev = (TS3.begin() + 4 * TS3.size() / 5)->taylor_sensitivity_3 + 0.1f;
        TS1.reserve(TS3.size());
        for (auto iter = TS3.begin(); iter != TS3.end(); iter++) {
            if (iter->taylor_sensitivity_3 <= temp_dev) { TS1.push_back(TS1_Pairs(*iter)); }
            else { break; }
        }
        TS3.clear();

        std::sort(TS1.begin(), TS1.end());
        temp_dev = (TS1.begin() + 4 * TS1.size() / 5)->taylor_sensitivity_1 + 0.1f;
        TS2.reserve(TS1.size());
        for (auto iter = TS1.begin(); iter != TS1.end(); iter++) {
            if (iter->taylor_sensitivity_1 <= temp_dev) { TS2.push_back(TS2_Pairs(*iter)); }
            else { break; }
        }
        TS1.clear();

        std::sort(TS2.begin(), TS2.end());
        temp_dev = (TS2.begin() + TS2.size() / 8)->taylor_sensitivity_2 + 0.1f;
        PDev.reserve(TS2.size());
        for (auto iter = TS2.begin(); iter != TS2.end(); iter++) {
            if (iter->taylor_sensitivity_2 <= temp_dev) { PDev.push_back(PDev_Pairs(*iter)); }
            else { break; }
        }
        TS2.clear();
    } else {
        PDev.reserve(TS3.size());
        for (auto iter = TS3.begin(); iter != TS3.end(); iter++) {
            PDev.push_back(PDev_Pairs(*iter));
        }
        TS3.clear();
    }

    if (y_params.empty_param || all_negative) {
        std::sort(PDev.begin(), PDev.end());
        temp_dev = (PDev.begin() + PDev.size() / 2)->pos_dev - 0.001f;
        MDev.reserve(PDev.size());
        for (auto iter = PDev.begin(); iter != PDev.end(); iter++) {
            if (iter->pos_dev >= temp_dev) { MDev.push_back(MDev_Pairs(*iter)); }
            else { break; }
        }
        PDev.clear();

        std::sort(MDev.begin(), MDev.end());
        temp_dev = (MDev.begin() + MDev.size() / 2)->mov_dev - 0.001f;
        PRDev.reserve(MDev.size());
        for (auto iter = MDev.begin(); iter != MDev.end(); iter++) {
            if (iter->mov_dev >= temp_dev) { PRDev.push_back(PRDev_Pairs(*iter)); }
            else { break; }
        }
        MDev.clear();

        std::sort(PRDev.begin(), PRDev.end());
        temp_dev = (PRDev.begin() + PRDev.size() / 2)->prof_dev - 0.001f;
        PNL.reserve(PRDev.size());
        for (auto iter = PRDev.begin(); iter != PRDev.end(); iter++) {
            if (iter->prof_dev >= temp_dev) { PNL.push_back(Pnl_Sorting_Pairs(*iter)); }
            else { break; }
        }
        PRDev.clear();
    } else {
        PNL.reserve(PDev.size());
        for (auto iter = PDev.begin(); iter != PDev.end(); iter++) {
            PNL.push_back(Pnl_Sorting_Pairs(*iter));
        }
        PDev.clear();
    }

    std::sort(PNL.begin(), PNL.end());
    y_params.fill(y_params.strategy_variant, lattice_points[PNL.front().position]->pos_param,
                  lattice_points[PNL.front().position]->mov_param,
                  lattice_points[PNL.front().position]->prof_param);

    return Numerical_Sim_Results(PNL.front().position);
}

void Numerical_Solver::Update_Yesterday_Params(const Yesterday_Params &other) {
    y_params.fill(other.strategy_variant, other.pos_param, other.mov_param, other.prof_param);
}

void Numerical_Solver::Update_Expiry_Params(const Yesterday_Params &other) {
    y_params.fill(y_params.strategy_variant, other.pos_param, other.mov_param, other.prof_param);
}

void Numerical_Solver::Update_Metadata(Numerical_Sim_Results &num_sim) const {

    vector<int> PNL, Positive_PNL, Negative_PNL;
    PNL.reserve((pos_length - 6) * (mov_length - 6) * (prof_length - 6));
    Positive_PNL.reserve(PNL.capacity());
    Negative_PNL.reserve(PNL.capacity());
    float pdiff_mean = 0.0f, mdiff_mean = 0.0f, prdiff_mean = 0.0f, pmprdiff_mean = 0.0f, pdiff_stdev = 0.0f,
            mdiff_stdev = 0.0f, prdiff_stdev = 0.0f, pmprdiff_stdev = 0.0f;

    num_sim.taylor_sensitivity_1 = lattice_points[num_sim.position]->taylor_sensitivity_1;
    num_sim.taylor_sensitivity_2 = lattice_points[num_sim.position]->taylor_sensitivity_2;
    num_sim.taylor_sensitivity_3 = lattice_points[num_sim.position]->taylor_sensitivity_3;
    num_sim.pos_dev = *lattice_points[num_sim.position]->pos_dev;
    num_sim.mov_dev = *lattice_points[num_sim.position]->mov_dev;
    num_sim.prof_dev = *lattice_points[num_sim.position]->prof_dev;
    num_sim.pos_diff = lattice_points[num_sim.position]->pos_diff;
    num_sim.mov_diff = lattice_points[num_sim.position]->mov_diff;
    num_sim.prof_diff = lattice_points[num_sim.position]->prof_diff;
    num_sim.pmpr_diff = lattice_points[num_sim.position]->pmpr_diff;

    int position = 0;
    for (int pos_counter = 3; pos_counter < pos_length - 3; pos_counter++) {
        for (int mov_counter = 3; mov_counter < mov_length - 3; mov_counter++) {
            for (int prof_counter = 3; prof_counter < prof_length - 3; prof_counter++) {
                position = pos_counter * (mov_length * prof_length) + mov_counter * (prof_length) + prof_counter;
                PNL.push_back(position);
                num_sim.variant_pnl_mean += lattice_points[position]->pnl;

                if (lattice_points[position]->pnl > 0.0f) {
                    Positive_PNL.push_back(position);
                    num_sim.variant_positive_pnl_mean += lattice_points[position]->pnl;
                } else {
                    Negative_PNL.push_back(position);
                    num_sim.variant_negative_pnl_mean += lattice_points[position]->pnl;
                }

                if (lattice_points[position]->drawdown_count < 2) { num_sim.variant_fav_drawdown_percent += 1.0f; }

                num_sim.variant_ts1_mean += lattice_points[position]->taylor_sensitivity_1;
                num_sim.variant_ts2_mean += lattice_points[position]->taylor_sensitivity_2;
                num_sim.variant_ts3_mean += lattice_points[position]->taylor_sensitivity_3;
                num_sim.variant_pdev_mean += *lattice_points[position]->pos_dev;
                num_sim.variant_mdev_mean += *lattice_points[position]->mov_dev;
                num_sim.variant_prdev_mean += *lattice_points[position]->prof_dev;
                pdiff_mean += lattice_points[position]->pos_diff;
                mdiff_mean += lattice_points[position]->mov_diff;
                prdiff_mean += lattice_points[position]->prof_diff;
                pmprdiff_mean += lattice_points[position]->pmpr_diff;
            }
        }
    }
    Positive_PNL.shrink_to_fit();
    Negative_PNL.shrink_to_fit();

    if (PNL.size() > 0) {
        num_sim.variant_profit_percent = (float) Positive_PNL.size() / PNL.size();
        num_sim.variant_pnl_mean /= PNL.size();
        num_sim.variant_fav_drawdown_percent /= PNL.size();
        num_sim.variant_ts1_mean /= PNL.size();
        num_sim.variant_ts2_mean /= PNL.size();
        num_sim.variant_ts3_mean /= PNL.size();
        num_sim.variant_pdev_mean /= PNL.size();
        num_sim.variant_mdev_mean /= PNL.size();
        num_sim.variant_prdev_mean /= PNL.size();
        pdiff_mean /= PNL.size();
        mdiff_mean /= PNL.size();
        prdiff_mean /= PNL.size();
        pmprdiff_mean /= PNL.size();
    } else {
        num_sim.variant_profit_percent = 0.0f;
        num_sim.variant_pnl_mean = 0.0f;
        num_sim.variant_fav_drawdown_percent = 0.0f;
        num_sim.variant_ts1_mean = 0.0f;
        num_sim.variant_ts2_mean = 0.0f;
        num_sim.variant_ts3_mean = 0.0f;
        num_sim.variant_pdev_mean = 0.0f;
        num_sim.variant_mdev_mean = 0.0f;
        num_sim.variant_prdev_mean = 0.0f;
        pdiff_mean = 0.0f;
        mdiff_mean = 0.0f;
        prdiff_mean = 0.0f;
        pmprdiff_mean = 0.0f;
    }

    if (Positive_PNL.size() > 0) { num_sim.variant_positive_pnl_mean /= Positive_PNL.size(); }
    else { num_sim.variant_positive_pnl_mean = 0.0f; }

    if (Negative_PNL.size() > 0) { num_sim.variant_negative_pnl_mean /= Negative_PNL.size(); }
    else { num_sim.variant_negative_pnl_mean = 0.0f; }

    for (auto &position: PNL) {
        num_sim.variant_pnl_stdev += powf(lattice_points[position]->pnl - num_sim.variant_pnl_mean, 2.0f);
        num_sim.variant_pnl_skew += powf(lattice_points[position]->pnl - num_sim.variant_pnl_mean, 3.0f);
        num_sim.variant_pnl_kurtosis += powf(lattice_points[position]->pnl - num_sim.variant_pnl_mean, 4.0f);
        num_sim.variant_ts1_stdev += powf(lattice_points[position]->taylor_sensitivity_1 - num_sim.variant_ts1_mean,
                                          2.0f);
        num_sim.variant_ts2_stdev += powf(lattice_points[position]->taylor_sensitivity_2 - num_sim.variant_ts2_mean,
                                          2.0f);
        num_sim.variant_ts3_stdev += powf(lattice_points[position]->taylor_sensitivity_3 - num_sim.variant_ts3_mean,
                                          2.0f);
        num_sim.variant_pdev_stdev += powf(*lattice_points[position]->pos_dev - num_sim.variant_pdev_mean, 2.0f);
        num_sim.variant_mdev_stdev += powf(*lattice_points[position]->mov_dev - num_sim.variant_mdev_mean, 2.0f);
        num_sim.variant_prdev_stdev += powf(*lattice_points[position]->prof_dev - num_sim.variant_prdev_mean, 2.0f);
        pdiff_stdev += powf(lattice_points[position]->pos_diff - pdiff_mean, 2.0f);
        mdiff_stdev += powf(lattice_points[position]->mov_diff - mdiff_mean, 2.0f);
        prdiff_stdev += powf(lattice_points[position]->prof_diff - prdiff_mean, 2.0f);
        pmprdiff_stdev += powf(lattice_points[position]->pmpr_diff - pmprdiff_mean, 2.0f);
    }

    for (auto &position: Positive_PNL) {
        num_sim.variant_positive_pnl_stdev += powf(lattice_points[position]->pnl - num_sim.variant_positive_pnl_mean,
                                                   2);
        num_sim.variant_positive_pnl_skew += powf(lattice_points[position]->pnl - num_sim.variant_positive_pnl_mean, 3);
        num_sim.variant_positive_pnl_kurtosis += powf(lattice_points[position]->pnl - num_sim.variant_positive_pnl_mean,
                                                      4);
    }

    for (auto &position: Negative_PNL) {
        num_sim.variant_negative_pnl_stdev += powf(lattice_points[position]->pnl - num_sim.variant_negative_pnl_mean,
                                                   2);
        num_sim.variant_negative_pnl_skew += powf(lattice_points[position]->pnl - num_sim.variant_negative_pnl_mean, 3);
        num_sim.variant_negative_pnl_kurtosis += powf(lattice_points[position]->pnl - num_sim.variant_negative_pnl_mean,
                                                      4);
    }

    if (PNL.size() > 0) {
        num_sim.variant_pnl_stdev = powf(num_sim.variant_pnl_stdev / PNL.size(), 0.5f);
        num_sim.variant_pnl_skew = cbrtf(num_sim.variant_pnl_skew / PNL.size());
        num_sim.variant_pnl_kurtosis = sqrtf(sqrtf(num_sim.variant_pnl_kurtosis / PNL.size()));
        num_sim.variant_ts1_stdev = powf(num_sim.variant_ts1_stdev / PNL.size(), 0.5f);
        num_sim.variant_ts2_stdev = powf(num_sim.variant_ts2_stdev / PNL.size(), 0.5f);
        num_sim.variant_ts3_stdev = powf(num_sim.variant_ts3_stdev / PNL.size(), 0.5f);
        num_sim.variant_pdev_stdev = powf(num_sim.variant_pdev_stdev / PNL.size(), 0.5f);
        num_sim.variant_mdev_stdev = powf(num_sim.variant_mdev_stdev / PNL.size(), 0.5f);
        num_sim.variant_prdev_stdev = powf(num_sim.variant_prdev_stdev / PNL.size(), 0.5f);
        pdiff_stdev = powf(pdiff_stdev / PNL.size(), 0.5f);
        mdiff_stdev = powf(mdiff_stdev / PNL.size(), 0.5f);
        prdiff_stdev = powf(prdiff_stdev / PNL.size(), 0.5f);
        pmprdiff_stdev = powf(pmprdiff_stdev / PNL.size(), 0.5f);
    } else {
        num_sim.variant_pnl_stdev = 0.0f;
        num_sim.variant_pnl_skew = 0.0f;
        num_sim.variant_pnl_kurtosis = 0.0f;
        num_sim.variant_ts1_stdev = 0.0f;
        num_sim.variant_ts2_stdev = 0.0f;
        num_sim.variant_ts3_stdev = 0.0f;
        num_sim.variant_pdev_stdev = 0.0f;
        num_sim.variant_mdev_stdev = 0.0f;
        num_sim.variant_prdev_stdev = 0.0f;
        pdiff_stdev = 0.0f;
        mdiff_stdev = 0.0f;
        prdiff_stdev = 0.0f;
        pmprdiff_stdev = 0.0f;
    }

    if (Positive_PNL.size() > 0) {
        num_sim.variant_positive_pnl_stdev = powf(num_sim.variant_positive_pnl_stdev / Positive_PNL.size(), 0.5f);
        num_sim.variant_positive_pnl_skew = cbrtf(num_sim.variant_positive_pnl_skew / Positive_PNL.size());
        num_sim.variant_positive_pnl_kurtosis = sqrtf(
                sqrtf(num_sim.variant_positive_pnl_kurtosis / Positive_PNL.size()));
    } else {
        num_sim.variant_positive_pnl_stdev = 0.0f;
        num_sim.variant_positive_pnl_skew = 0.0f;
        num_sim.variant_positive_pnl_kurtosis = 0.0f;
    }
    Positive_PNL.clear();

    if (Negative_PNL.size() > 0) {
        num_sim.variant_negative_pnl_stdev = powf(num_sim.variant_negative_pnl_stdev / Negative_PNL.size(), 0.5f);
        num_sim.variant_negative_pnl_skew = cbrtf(num_sim.variant_negative_pnl_skew / Negative_PNL.size());
        num_sim.variant_negative_pnl_kurtosis = sqrtf(
                sqrtf(num_sim.variant_negative_pnl_kurtosis / Negative_PNL.size()));
    } else {
        num_sim.variant_negative_pnl_stdev = 0.0f;
        num_sim.variant_negative_pnl_skew = 0.0f;
        num_sim.variant_negative_pnl_kurtosis = 0.0f;
    }
    Negative_PNL.clear();

    for (auto &position: PNL) {
        num_sim.variant_ts1_pnl_corr += (lattice_points[position]->pnl - num_sim.variant_pnl_mean) *
                                        (lattice_points[position]->taylor_sensitivity_1 - num_sim.variant_ts1_mean);
        num_sim.variant_ts2_pnl_corr += (lattice_points[position]->pnl - num_sim.variant_pnl_mean) *
                                        (lattice_points[position]->taylor_sensitivity_2 - num_sim.variant_ts2_mean);
        num_sim.variant_ts3_pnl_corr += (lattice_points[position]->pnl - num_sim.variant_pnl_mean) *
                                        (lattice_points[position]->taylor_sensitivity_3 - num_sim.variant_ts3_mean);
        num_sim.variant_ts1_pdev_corr += (*lattice_points[position]->pos_dev - num_sim.variant_pdev_mean) *
                                         (lattice_points[position]->taylor_sensitivity_1 - num_sim.variant_ts1_mean);
        num_sim.variant_ts1_mdev_corr += (*lattice_points[position]->mov_dev - num_sim.variant_mdev_mean) *
                                         (lattice_points[position]->taylor_sensitivity_1 - num_sim.variant_ts1_mean);
        num_sim.variant_ts1_prdev_corr += (*lattice_points[position]->prof_dev - num_sim.variant_prdev_mean) *
                                          (lattice_points[position]->taylor_sensitivity_1 - num_sim.variant_ts1_mean);
        num_sim.variant_ts2_pdev_corr += (*lattice_points[position]->pos_dev - num_sim.variant_pdev_mean) *
                                         (lattice_points[position]->taylor_sensitivity_2 - num_sim.variant_ts2_mean);
        num_sim.variant_ts2_mdev_corr += (*lattice_points[position]->mov_dev - num_sim.variant_mdev_mean) *
                                         (lattice_points[position]->taylor_sensitivity_2 - num_sim.variant_ts2_mean);
        num_sim.variant_ts2_prdev_corr += (*lattice_points[position]->prof_dev - num_sim.variant_prdev_mean) *
                                          (lattice_points[position]->taylor_sensitivity_2 - num_sim.variant_ts2_mean);
        num_sim.variant_ts3_pdev_corr += (*lattice_points[position]->pos_dev - num_sim.variant_pdev_mean) *
                                         (lattice_points[position]->taylor_sensitivity_3 - num_sim.variant_ts3_mean);
        num_sim.variant_ts3_mdev_corr += (*lattice_points[position]->mov_dev - num_sim.variant_mdev_mean) *
                                         (lattice_points[position]->taylor_sensitivity_3 - num_sim.variant_ts3_mean);
        num_sim.variant_ts3_prdev_corr += (*lattice_points[position]->prof_dev - num_sim.variant_prdev_mean) *
                                          (lattice_points[position]->taylor_sensitivity_3 - num_sim.variant_ts3_mean);
        num_sim.variant_pnl_pdiff_corr += (lattice_points[position]->pnl - num_sim.variant_pnl_mean) *
                                          (lattice_points[position]->pos_diff - pdiff_mean);
        num_sim.variant_pnl_mdiff_corr += (lattice_points[position]->pnl - num_sim.variant_pnl_mean) *
                                          (lattice_points[position]->mov_diff - mdiff_mean);
        num_sim.variant_pnl_prdiff_corr += (lattice_points[position]->pnl - num_sim.variant_pnl_mean) *
                                           (lattice_points[position]->prof_diff - prdiff_mean);
        num_sim.variant_pnl_pmprdiff_corr += (lattice_points[position]->pnl - num_sim.variant_pnl_mean) *
                                             (lattice_points[position]->pmpr_diff - pmprdiff_mean);
        num_sim.variant_ts1_pdiff_corr += (lattice_points[position]->taylor_sensitivity_1 - num_sim.variant_ts1_mean) *
                                          (lattice_points[position]->pos_diff - pdiff_mean);
        num_sim.variant_ts1_mdiff_corr += (lattice_points[position]->taylor_sensitivity_1 - num_sim.variant_ts1_mean) *
                                          (lattice_points[position]->mov_diff - mdiff_mean);
        num_sim.variant_ts1_prdiff_corr += (lattice_points[position]->taylor_sensitivity_1 - num_sim.variant_ts1_mean) *
                                           (lattice_points[position]->prof_diff - prdiff_mean);
        num_sim.variant_ts1_pmprdiff_corr +=
                (lattice_points[position]->taylor_sensitivity_1 - num_sim.variant_ts1_mean) *
                (lattice_points[position]->pmpr_diff - pmprdiff_mean);
        num_sim.variant_ts2_pdiff_corr += (lattice_points[position]->taylor_sensitivity_2 - num_sim.variant_ts2_mean) *
                                          (lattice_points[position]->pos_diff - pdiff_mean);
        num_sim.variant_ts2_mdiff_corr += (lattice_points[position]->taylor_sensitivity_2 - num_sim.variant_ts2_mean) *
                                          (lattice_points[position]->mov_diff - mdiff_mean);
        num_sim.variant_ts2_prdiff_corr += (lattice_points[position]->taylor_sensitivity_2 - num_sim.variant_ts2_mean) *
                                           (lattice_points[position]->prof_diff - prdiff_mean);
        num_sim.variant_ts2_pmprdiff_corr +=
                (lattice_points[position]->taylor_sensitivity_2 - num_sim.variant_ts2_mean) *
                (lattice_points[position]->pmpr_diff - pmprdiff_mean);
        num_sim.variant_ts3_pdiff_corr += (lattice_points[position]->taylor_sensitivity_3 - num_sim.variant_ts3_mean) *
                                          (lattice_points[position]->pos_diff - pdiff_mean);
        num_sim.variant_ts3_mdiff_corr += (lattice_points[position]->taylor_sensitivity_3 - num_sim.variant_ts3_mean) *
                                          (lattice_points[position]->mov_diff - mdiff_mean);
        num_sim.variant_ts3_prdiff_corr += (lattice_points[position]->taylor_sensitivity_3 - num_sim.variant_ts3_mean) *
                                           (lattice_points[position]->prof_diff - prdiff_mean);
        num_sim.variant_ts3_pmprdiff_corr +=
                (lattice_points[position]->taylor_sensitivity_3 - num_sim.variant_ts3_mean) *
                (lattice_points[position]->pmpr_diff - pmprdiff_mean);
        num_sim.variant_pdev_pdiff_corr += (*lattice_points[position]->pos_dev - num_sim.variant_pdev_mean) *
                                           (lattice_points[position]->pos_diff - pdiff_mean);
        num_sim.variant_pdev_mdiff_corr += (*lattice_points[position]->pos_dev - num_sim.variant_pdev_mean) *
                                           (lattice_points[position]->mov_diff - mdiff_mean);
        num_sim.variant_pdev_prdiff_corr += (*lattice_points[position]->pos_dev - num_sim.variant_pdev_mean) *
                                            (lattice_points[position]->prof_diff - prdiff_mean);
        num_sim.variant_pdev_pmprdiff_corr += (*lattice_points[position]->pos_dev - num_sim.variant_pdev_mean) *
                                              (lattice_points[position]->pmpr_diff - pmprdiff_mean);
        num_sim.variant_mdev_pdiff_corr += (*lattice_points[position]->mov_dev - num_sim.variant_mdev_mean) *
                                           (lattice_points[position]->pos_diff - pdiff_mean);
        num_sim.variant_mdev_mdiff_corr += (*lattice_points[position]->mov_dev - num_sim.variant_mdev_mean) *
                                           (lattice_points[position]->mov_diff - mdiff_mean);
        num_sim.variant_mdev_prdiff_corr += (*lattice_points[position]->mov_dev - num_sim.variant_mdev_mean) *
                                            (lattice_points[position]->prof_diff - prdiff_mean);
        num_sim.variant_mdev_pmprdiff_corr += (*lattice_points[position]->mov_dev - num_sim.variant_mdev_mean) *
                                              (lattice_points[position]->pmpr_diff - pmprdiff_mean);
        num_sim.variant_prdev_pdiff_corr += (*lattice_points[position]->prof_dev - num_sim.variant_prdev_mean) *
                                            (lattice_points[position]->pos_diff - pdiff_mean);
        num_sim.variant_prdev_mdiff_corr += (*lattice_points[position]->prof_dev - num_sim.variant_prdev_mean) *
                                            (lattice_points[position]->mov_diff - mdiff_mean);
        num_sim.variant_prdev_prdiff_corr += (*lattice_points[position]->prof_dev - num_sim.variant_prdev_mean) *
                                             (lattice_points[position]->prof_diff - prdiff_mean);
        num_sim.variant_prdev_pmprdiff_corr += (*lattice_points[position]->prof_dev - num_sim.variant_prdev_mean) *
                                               (lattice_points[position]->pmpr_diff - pmprdiff_mean);
    }

    if (num_sim.variant_pnl_stdev > 0.0f && num_sim.variant_ts1_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts1_pnl_corr /= (PNL.size() * num_sim.variant_pnl_stdev * num_sim.variant_ts1_stdev);
    } else { num_sim.variant_ts1_pnl_corr = 0.0f; }

    if (num_sim.variant_pnl_stdev > 0.0f && num_sim.variant_ts2_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts2_pnl_corr /= (PNL.size() * num_sim.variant_pnl_stdev * num_sim.variant_ts2_stdev);
    } else { num_sim.variant_ts2_pnl_corr = 0.0f; }

    if (num_sim.variant_pnl_stdev > 0.0f && num_sim.variant_ts3_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts3_pnl_corr /= (PNL.size() * num_sim.variant_pnl_stdev * num_sim.variant_ts3_stdev);
    } else { num_sim.variant_ts3_pnl_corr = 0.0f; }

    if (num_sim.variant_pdev_stdev > 0.0f && num_sim.variant_ts1_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts1_pdev_corr /= (PNL.size() * num_sim.variant_pdev_stdev * num_sim.variant_ts1_stdev);
    } else { num_sim.variant_ts1_pdev_corr = 0.0f; }

    if (num_sim.variant_mdev_stdev > 0.0f && num_sim.variant_ts1_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts1_mdev_corr /= (PNL.size() * num_sim.variant_mdev_stdev * num_sim.variant_ts1_stdev);
    } else { num_sim.variant_ts1_mdev_corr = 0.0f; }

    if (num_sim.variant_prdev_stdev > 0.0f && num_sim.variant_ts1_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts1_prdev_corr /= (PNL.size() * num_sim.variant_prdev_stdev * num_sim.variant_ts1_stdev);
    } else { num_sim.variant_ts1_prdev_corr = 0.0f; }

    if (num_sim.variant_pdev_stdev > 0.0f && num_sim.variant_ts2_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts2_pdev_corr /= (PNL.size() * num_sim.variant_pdev_stdev * num_sim.variant_ts2_stdev);
    } else { num_sim.variant_ts2_pdev_corr = 0.0f; }

    if (num_sim.variant_mdev_stdev > 0.0f && num_sim.variant_ts2_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts2_mdev_corr /= (PNL.size() * num_sim.variant_mdev_stdev * num_sim.variant_ts2_stdev);
    } else { num_sim.variant_ts2_mdev_corr = 0.0f; }

    if (num_sim.variant_prdev_stdev > 0.0f && num_sim.variant_ts2_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts2_prdev_corr /= (PNL.size() * num_sim.variant_prdev_stdev * num_sim.variant_ts2_stdev);
    } else { num_sim.variant_ts2_prdev_corr = 0.0f; }

    if (num_sim.variant_pdev_stdev > 0.0f && num_sim.variant_ts3_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts3_pdev_corr /= (PNL.size() * num_sim.variant_pdev_stdev * num_sim.variant_ts3_stdev);
    } else { num_sim.variant_ts3_pdev_corr = 0.0f; }

    if (num_sim.variant_mdev_stdev > 0.0f && num_sim.variant_ts3_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts3_mdev_corr /= (PNL.size() * num_sim.variant_mdev_stdev * num_sim.variant_ts3_stdev);
    } else { num_sim.variant_ts3_mdev_corr = 0.0f; }

    if (num_sim.variant_prdev_stdev > 0.0f && num_sim.variant_ts3_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts3_prdev_corr /= (PNL.size() * num_sim.variant_prdev_stdev * num_sim.variant_ts3_stdev);
    } else { num_sim.variant_ts3_prdev_corr = 0.0f; }

    if (num_sim.variant_pnl_stdev > 0.0f && pdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_pnl_pdiff_corr /= (PNL.size() * num_sim.variant_pnl_stdev * pdiff_stdev);
    } else { num_sim.variant_pnl_pdiff_corr = 0.0f; }

    if (num_sim.variant_pnl_stdev > 0.0f && mdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_pnl_mdiff_corr /= (PNL.size() * num_sim.variant_pnl_stdev * mdiff_stdev);
    } else { num_sim.variant_pnl_mdiff_corr = 0.0f; }

    if (num_sim.variant_pnl_stdev > 0.0f && prdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_pnl_prdiff_corr /= (PNL.size() * num_sim.variant_pnl_stdev * prdiff_stdev);
    } else { num_sim.variant_pnl_prdiff_corr = 0.0f; }

    if (num_sim.variant_pnl_stdev > 0.0f && pmprdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_pnl_pmprdiff_corr /= (PNL.size() * num_sim.variant_pnl_stdev * pmprdiff_stdev);
    } else { num_sim.variant_pnl_pmprdiff_corr = 0.0f; }

    if (num_sim.variant_ts1_stdev > 0.0f && pdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts1_pdiff_corr /= (PNL.size() * num_sim.variant_ts1_stdev * pdiff_stdev);
    } else { num_sim.variant_ts1_pdiff_corr = 0.0f; }

    if (num_sim.variant_ts1_stdev > 0.0f && mdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts1_mdiff_corr /= (PNL.size() * num_sim.variant_ts1_stdev * mdiff_stdev);
    } else { num_sim.variant_ts1_mdiff_corr = 0.0f; }

    if (num_sim.variant_ts1_stdev > 0.0f && prdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts1_prdiff_corr /= (PNL.size() * num_sim.variant_ts1_stdev * prdiff_stdev);
    } else { num_sim.variant_ts1_prdiff_corr = 0.0f; }

    if (num_sim.variant_ts1_stdev > 0.0f && pmprdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts1_pmprdiff_corr /= (PNL.size() * num_sim.variant_ts1_stdev * pmprdiff_stdev);
    } else { num_sim.variant_ts1_pmprdiff_corr = 0.0f; }

    if (num_sim.variant_ts2_stdev > 0.0f && pdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts2_pdiff_corr /= (PNL.size() * num_sim.variant_ts2_stdev * pdiff_stdev);
    } else { num_sim.variant_ts2_pdiff_corr = 0.0f; }

    if (num_sim.variant_ts2_stdev > 0.0f && mdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts2_mdiff_corr /= (PNL.size() * num_sim.variant_ts2_stdev * mdiff_stdev);
    } else { num_sim.variant_ts2_mdiff_corr = 0.0f; }

    if (num_sim.variant_ts2_stdev > 0.0f && prdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts2_prdiff_corr /= (PNL.size() * num_sim.variant_ts2_stdev * prdiff_stdev);
    } else { num_sim.variant_ts2_prdiff_corr = 0.0f; }

    if (num_sim.variant_ts2_stdev > 0.0f && pmprdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts2_pmprdiff_corr /= (PNL.size() * num_sim.variant_ts2_stdev * pmprdiff_stdev);
    } else { num_sim.variant_ts2_pmprdiff_corr = 0.0f; }

    if (num_sim.variant_ts3_stdev > 0.0f && pdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts3_pdiff_corr /= (PNL.size() * num_sim.variant_ts3_stdev * pdiff_stdev);
    } else { num_sim.variant_ts3_pdiff_corr = 0.0f; }

    if (num_sim.variant_ts3_stdev > 0.0f && mdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts3_mdiff_corr /= (PNL.size() * num_sim.variant_ts3_stdev * mdiff_stdev);
    } else { num_sim.variant_ts3_mdiff_corr = 0.0f; }

    if (num_sim.variant_ts3_stdev > 0.0f && prdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts3_prdiff_corr /= (PNL.size() * num_sim.variant_ts3_stdev * prdiff_stdev);
    } else { num_sim.variant_ts3_prdiff_corr = 0.0f; }

    if (num_sim.variant_ts3_stdev > 0.0f && pmprdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_ts3_pmprdiff_corr /= (PNL.size() * num_sim.variant_ts3_stdev * pmprdiff_stdev);
    } else { num_sim.variant_ts3_pmprdiff_corr = 0.0f; }

    if (num_sim.variant_pdev_stdev > 0.0f && pdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_pdev_pdiff_corr /= (PNL.size() * num_sim.variant_pdev_stdev * pdiff_stdev);
    } else { num_sim.variant_pdev_pdiff_corr = 0.0f; }

    if (num_sim.variant_pdev_stdev > 0.0f && mdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_pdev_mdiff_corr /= (PNL.size() * num_sim.variant_pdev_stdev * mdiff_stdev);
    } else { num_sim.variant_pdev_mdiff_corr = 0.0f; }

    if (num_sim.variant_pdev_stdev > 0.0f && prdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_pdev_prdiff_corr /= (PNL.size() * num_sim.variant_pdev_stdev * prdiff_stdev);
    } else { num_sim.variant_pdev_prdiff_corr = 0.0f; }

    if (num_sim.variant_pdev_stdev > 0.0f && pmprdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_pdev_pmprdiff_corr /= (PNL.size() * num_sim.variant_pdev_stdev * pmprdiff_stdev);
    } else { num_sim.variant_pdev_pmprdiff_corr = 0.0f; }

    if (num_sim.variant_mdev_stdev > 0.0f && pdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_mdev_pdiff_corr /= (PNL.size() * num_sim.variant_mdev_stdev * pdiff_stdev);
    } else { num_sim.variant_mdev_pdiff_corr = 0.0f; }

    if (num_sim.variant_mdev_stdev > 0.0f && mdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_mdev_mdiff_corr /= (PNL.size() * num_sim.variant_mdev_stdev * mdiff_stdev);
    } else { num_sim.variant_mdev_mdiff_corr = 0.0f; }

    if (num_sim.variant_mdev_stdev > 0.0f && prdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_mdev_prdiff_corr /= (PNL.size() * num_sim.variant_mdev_stdev * prdiff_stdev);
    } else { num_sim.variant_mdev_prdiff_corr = 0.0f; }

    if (num_sim.variant_mdev_stdev > 0.0f && pmprdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_mdev_pmprdiff_corr /= (PNL.size() * num_sim.variant_mdev_stdev * pmprdiff_stdev);
    } else { num_sim.variant_mdev_pmprdiff_corr = 0.0f; }

    if (num_sim.variant_prdev_stdev > 0.0f && pdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_prdev_pdiff_corr /= (PNL.size() * num_sim.variant_prdev_stdev * pdiff_stdev);
    } else { num_sim.variant_prdev_pdiff_corr = 0.0f; }

    if (num_sim.variant_prdev_stdev > 0.0f && mdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_prdev_mdiff_corr /= (PNL.size() * num_sim.variant_prdev_stdev * mdiff_stdev);
    } else { num_sim.variant_prdev_mdiff_corr = 0.0f; }

    if (num_sim.variant_prdev_stdev > 0.0f && prdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_prdev_prdiff_corr /= (PNL.size() * num_sim.variant_prdev_stdev * prdiff_stdev);
    } else { num_sim.variant_prdev_prdiff_corr = 0.0f; }

    if (num_sim.variant_prdev_stdev > 0.0f && pmprdiff_stdev > 0.0f && PNL.size() > 0) {
        num_sim.variant_prdev_pmprdiff_corr /= (PNL.size() * num_sim.variant_prdev_stdev * pmprdiff_stdev);
    } else { num_sim.variant_prdev_pmprdiff_corr = 0.0f; }
}

Yesterday_Params Numerical_Solver::Get_Yesterday_Params() const {
    return y_params;
}

#endif
