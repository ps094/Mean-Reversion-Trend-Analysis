#ifndef DIRECTIONAL_SIGNAL_GENERATOR_CPP
#define DIRECTIONAL_SIGNAL_GENERATOR_CPP

#include "Directional_Signal_Generator.h"

Directional_Signal_Generator::Directional_Signal_Generator(const Strategy_Parameters &Strategy_Params) {
    strategy_type = Strategy_Params.strategy_type;
    strategy_version = Strategy_Params.strategy_version;
    closing_prices.push_back(Strategy_Params.previous_close);
    critical_point = Strategy_Params.previous_close;
    critical_index = 0;
    close = Strategy_Params.previous_close;
    direction = 0;
    signal = 0;
    initial_move_flag = 0;
    current_batch_id = 0;
    position_parameter = Strategy_Params.position_parameter;
    extreme_move_parameter = Strategy_Params.extreme_move_parameter;
    abstinence_parameter = Strategy_Params.abstinence_parameter;
    straight_abstinence_parameter = Strategy_Params.straight_abstinence_parameter;
    straight_reversal_parameter = Strategy_Params.straight_reversal_parameter;
    directional_move_parameter = Strategy_Params.directional_move_parameter;
    initial_move_parameter = Strategy_Params.initial_move_parameter;
    initial_time_parameter = Strategy_Params.initial_time_parameter;
    position_flag = 0;
    extreme_move_flag = 0;
    straight_move_flag = 0;
}

void Directional_Signal_Generator::_position_flag() {
    float percent_change = abs(100.0f * (close - critical_point) / critical_point);
    if (percent_change < position_parameter) {
        position_flag = 0;
    } else if (percent_change < abstinence_parameter) {
        position_flag = 1;
    } else {
        position_flag = 0;
    }
}

void Directional_Signal_Generator::_extreme_move_flag() {
    float percent_change = abs(100.0f * (close - critical_point) / critical_point);
    if (percent_change < extreme_move_parameter) {
        extreme_move_flag = 0;
    } else {
        extreme_move_flag = 1;
    }
}

int Directional_Signal_Generator::_up_move_indicator(const std::vector<float> &subset) {
    if (subset.size() > 1) {
        int index = 1;
        float lower = subset[0];
        float upper = subset[0];
        int upper_index = 0;
        int straight_flag = 0;
        float abstinence = straight_abstinence_parameter * critical_point / 100.0f;
        float reversal = straight_reversal_parameter * critical_point / 100.0f;
        while (index < subset.size()) {
            if (subset[index] >= upper) {
                upper = subset[index];
                upper_index = index;
            } else if ((upper - subset[index]) >= reversal) {
                if (straight_flag == 1) {
                    straight_flag = 0;
                    critical_point = subset[index];
                    critical_index += index;
                }
                lower = subset[index];
                upper = subset[index];
                upper_index = index;
            }
            if (straight_flag == 0 && (upper - lower) >= abstinence) {
                straight_flag = 1;
            }
            index += 1;
        }
        return straight_flag;
    } else {
        return 0;
    }
}

int Directional_Signal_Generator::_down_move_indicator(const std::vector<float> &subset) {
    if (subset.size() > 1) {
        int index = 1;
        float lower = subset[0];
        float upper = subset[0];
        int lower_index = 0;
        int straight_flag = 0;
        float abstinence = straight_abstinence_parameter * critical_point / 100.0f;
        float reversal = straight_reversal_parameter * critical_point / 100.0f;

        while (index < subset.size()) {
            if (subset[index] <= lower) {
                lower = subset[index];
                lower_index = index;
            } else if (subset[index] - lower >= reversal) {
                if (straight_flag == 1) {
                    straight_flag = 0;
                    critical_point = subset[index];
                    critical_index += index;
                }
                lower = subset[index];
                upper = subset[index];
                lower_index = index;
            }
            if (straight_flag == 0 && (upper - lower) >= abstinence) {
                straight_flag = 1;
            }
            index += 1;
        }
        return straight_flag;
    } else {
        return 0;
    }
}

void Directional_Signal_Generator::_straight_move_flag() {
    std::vector<float>::iterator first = closing_prices.begin();
    std::advance(first, critical_index);
    std::vector<float>::iterator last = closing_prices.end();
    std::vector<float> subset1(first, last);
    int up_signal = _up_move_indicator(subset1);

    if (up_signal == 1) {
        straight_move_flag = 1;
    } else {
        first = closing_prices.begin();
        std::advance(first, critical_index);
        last = closing_prices.end();
        std::vector<float> subset2(first, last);
        int down_signal = _down_move_indicator(subset2);
        if (down_signal == 1) {
            straight_move_flag = 1;
        } else {
            straight_move_flag = 0;
        }
    }
}

void Directional_Signal_Generator::_direction_flag() {
    std::vector<float>::iterator first = closing_prices.begin();
    std::advance(first, 1);
    std::vector<float>::iterator last = closing_prices.end();
    std::vector<float> subset(first, last);
    int _direction = 0;
    float initialmovequanta = initial_move_parameter * closing_prices[0] / 100.0f;

    if (subset.size() > 0) {
        float upper = subset[0];
        float lower = subset[0];
        int index = 1;
        float directionalmovequanta = directional_move_parameter * closing_prices[0] / 100.0f;

        while (index < subset.size()) {
            if (subset[index] > upper) {
                upper = subset[index];
            } else if (subset[index] < lower) {
                lower = subset[index];
            }
            if (subset[index] - lower >= directionalmovequanta) {
                _direction = 1;
                lower = subset[index];
            } else if (upper - subset[index] >= directionalmovequanta) {
                _direction = -1;
                upper = subset[index];
            }
            index += 1;
        }
    }

    if (_direction != 0) {
        direction = _direction;
    } else if (close - closing_prices[0] > initialmovequanta) {
        direction = 1;
        initial_move_flag = 1;
    } else if (closing_prices[0] - close > initialmovequanta) {
        direction = -1;
        initial_move_flag = -1;
    } else {
        direction = initial_move_flag;
    }
}

std::tuple<int, int>
Directional_Signal_Generator::Update(const int &batch_id, const float &price, const bool &critical) {
    if (batch_id > current_batch_id) {
        current_batch_id = batch_id;
        closing_prices.push_back(price);
        close = price;

        if (critical == true) {
            critical_point = price;
            critical_index = batch_id;
        }

        _direction_flag();
        _straight_move_flag();
        _extreme_move_flag();
        _position_flag();

        if (direction == 0) {
            signal = 0;
        } else if (extreme_move_flag == 1) {
            signal = 1;
        } else if (straight_move_flag == 0) {
            if (position_flag == 1) {
                signal = 1;
            } else {
                signal = 0;
            }
        } else {
            signal = 0;
        }

        if (strategy_version == 2) { direction *= -1; }

        if (current_batch_id >= initial_time_parameter) {
            return std::tuple<int, int>{direction, signal};
        } else {
            return std::tuple<int, int>{0, 0};
        }
    } else {
        return std::tuple<int, int>{-100, -100};
    }
}


#endif
