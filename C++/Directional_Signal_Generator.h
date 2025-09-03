#ifndef DIRECTIONAL_SIGNAL_GENERATOR_H
#define DIRECTIONAL_SIGNAL_GENERATOR_H

#include<vector>
#include<tuple>
#include<iostream>
#include "Utilities.h"
#include<iterator>

class Directional_Signal_Generator {
public:

    int strategy_type, strategy_version, initial_time_parameter, critical_index, direction, signal, initial_move_flag,
            current_batch_id, position_flag, extreme_move_flag, straight_move_flag;
    float position_parameter, extreme_move_parameter, abstinence_parameter,
            straight_abstinence_parameter, straight_reversal_parameter, directional_move_parameter,
            initial_move_parameter, critical_point, close;
    std::vector<float> closing_prices;

    Directional_Signal_Generator(const Strategy_Parameters &);

    Directional_Signal_Generator(const Directional_Signal_Generator &other) = default;

    Directional_Signal_Generator &operator=(const Directional_Signal_Generator &other) = default;

    void _position_flag();

    void _extreme_move_flag();

    int _up_move_indicator(const std::vector<float> &);

    int _down_move_indicator(const std::vector<float> &);

    void _straight_move_flag();

    void _direction_flag();

    std::tuple<int, int> Update(const int &, const float &, const bool &);
};

#endif // !DIRECTIONAL_SIGNAL_GENERATOR_H

