#ifndef DIRECTIONAL_STRATEGY_MANAGER_H
#define DIRECTIONAL_STRATEGY_MANAGER_H

#include <iomanip>
#include<iostream>
#include <map>
#include<vector>
#include<algorithm>
#include "DataFrame.h"
#include "Utilities.h"
#include "BS.h"
#include "Directional_Signal_Generator.h"

class Directional_Strategy_Manager {
private:
    time_t date_today;
    Strategy_Parameters strategy_params;
    std::string underlying;
    Directional_Signal_Generator Signal_Generator;

public:
    Directional_Strategy_Manager(const time_t &, const Strategy_Parameters &);

    Directional_Strategy_Manager(const Directional_Strategy_Manager &other) = default;

    Directional_Strategy_Manager &operator=(const Directional_Strategy_Manager &other) = default;

    Simulation_Results test(const std::vector<DOHLCI> &, const bool &, const float &, const float &,
                            const std::tuple<int, int> &);
};

#endif
