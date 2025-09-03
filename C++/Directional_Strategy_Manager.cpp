#ifndef DIRECTIONAL_STRATEGY_MANAGER_CPP
#define DIRECTIONAL_STRATEGY_MANAGER_CPP

#include "Directional_Strategy_Manager.h"

Directional_Strategy_Manager::Directional_Strategy_Manager(const time_t &Date_Today,
                                                           const Strategy_Parameters &Strategy_Params)
        : date_today(Date_Today), strategy_params(Strategy_Params),
          Signal_Generator(Directional_Signal_Generator(Strategy_Params)) {}

Simulation_Results Directional_Strategy_Manager::test(const std::vector<DOHLCI> &prices, const bool &current_week,
                                                      const float &strike_offset, const float &vol,
                                                      const std::tuple<int, int> &expiries) {
    Simulation_Results summary;
    DOHLCI row;
    BS bs;

    try {
        std::tuple<int, int> direction_signal(0, 0);
        int direction = 0, signal = 0, position_flag = 0, trade_batch = 0;
        float call_value = 0.0f, put_value = 0.0f, call_price = 0.0f, put_price = 0.0f, call_pos = 0.0f, put_pos = 0.0f, call_strike = 0.0f,
                put_strike = 0.0f, previous_close = 0.0f, constant = 0.0f, trade_time = 0.0f, time_to_expiry = 0.0f, exit_price = 0.0f,
                hedge_price = 0.0f, stop_loss = 0.0f, exit_time = 0.0f;


        if (current_week) { time_to_expiry = (float) std::get<0>(expiries); }
        else { time_to_expiry = (float) std::get<1>(expiries); }
        trade_time = time_to_expiry;

        for (int i = 0; i < 375; i++) {
            row = prices[i];

            if (row.batch_id > 344) {
                trade_time = time_to_expiry - row.batch_id / 375.0f;
                call_value = bs.bs_call(row.close, call_strike, trade_time / 252.0f, 0.07f, vol);
                put_value = bs.bs_put(row.close, put_strike, trade_time / 252.0f, 0.07f, vol);
                summary.capital += call_pos * call_value + put_pos * put_value;
                summary.pnl += call_pos * (call_value - call_price) + put_pos * (put_value - put_price);
                position_flag = 0;
                call_pos = 0.0f;
                put_pos = 0.0f;
                break;
            }

            if (direction == -100 || signal == -100) {
                std::cout << std::endl << "Same Batch ID Provided To Signal Generator";
                direction_signal = Signal_Generator.Update(row.batch_id, row.close, false);
                direction = std::get<0>(direction_signal);
                signal = std::get<1>(direction_signal);
                continue;
            }

            if (direction * signal != 0 && position_flag == 0) {
                trade_time = time_to_expiry - row.batch_id / 375.0f;
                trade_batch = row.batch_id;
                previous_close = Signal_Generator.close;
                call_strike = previous_close * (1.0f + strike_offset / 100.0f);
                put_strike = previous_close * (1.0f - strike_offset / 100.0f);
                call_price = bs.bs_call(previous_close, call_strike, trade_time / 252.0f, 0.07f, vol);
                put_price = bs.bs_put(previous_close, put_strike, trade_time / 252.0f, 0.07f, vol);
                constant = summary.capital > 100000.0f ? 100000.0f : summary.capital;

                if (direction > 0) {
                    constant /= (strategy_params.position_ratio * call_price) +
                                (1.0f - strategy_params.position_ratio) * put_price;
                    call_pos = constant * strategy_params.position_ratio;
                    put_pos = constant * (1.0f - strategy_params.position_ratio);
                    exit_price = previous_close * (1.0f + strategy_params.take_profit_parameter * 0.01f);
                    hedge_price = previous_close * (1.0f - strategy_params.hedge_manage_parameter * 0.01f);
                    stop_loss = previous_close * (1.0f - strategy_params.stop_loss_parameter * 0.01f);
                    position_flag = 1;
                } else {
                    constant /= (1.0f - strategy_params.position_ratio) * call_price +
                                (strategy_params.position_ratio * put_price);
                    call_pos = constant * (1.0f - strategy_params.position_ratio);
                    put_pos = constant * strategy_params.position_ratio;
                    exit_price = previous_close * (1.0f - strategy_params.take_profit_parameter * 0.01f);
                    hedge_price = previous_close * (1.0f + strategy_params.hedge_manage_parameter * 0.01f);
                    stop_loss = previous_close * (1.0f + strategy_params.stop_loss_parameter * 0.01f);
                    position_flag = -1;
                }

                summary.trade_count++;
                summary.capital -= call_pos * call_price + put_pos * put_price;
                direction_signal = Signal_Generator.Update(row.batch_id, row.close, false);
                direction = std::get<0>(direction_signal);
                signal = std::get<1>(direction_signal);
                continue;
            }

            if (position_flag > 0) {
                exit_time = trade_time - (row.batch_id - trade_batch) / 375.0f;

                if (row.low <= hedge_price) {
                    if (put_pos != 0.0f) {
                        put_value = bs.bs_put(hedge_price, put_strike, exit_time / 252.0f, 0.07f, vol);

                        if (strategy_params.strategy_type == 2 && put_pos != 0.0f) {
                            call_value = bs.bs_call(hedge_price, call_strike, exit_time / 252.0f, 0.07f, vol);
                            call_price = (call_price * call_pos + put_pos * put_value) /
                                         (call_pos + (put_pos * put_value / call_value));
                            call_pos += put_pos * put_value / call_value;
                        } else { summary.capital += put_pos * put_value; }

                        summary.pnl += put_pos * (put_value - put_price);
                        put_pos = 0.0f;
                    } else { put_value = 0.0f; }

                    if (row.low <= stop_loss) {
                        call_value = bs.bs_call(stop_loss, call_strike, exit_time / 252.0f, 0.07f, vol);
                        summary.capital += call_pos * call_value + put_pos * put_value;
                        summary.pnl += call_pos * (call_value - call_price) + put_pos * (put_value - put_price);
                        position_flag = 0;
                        call_pos = 0.0f;
                        put_pos = 0.0f;

                        if (summary.drawdown_count <= 1) {
                            if (summary.drawdown_count == 0) { summary.first_drawdown_pnl = summary.pnl; }
                            summary.second_drawdown_pnl = summary.pnl;
                        }

                        summary.drawdown_count++;
                        direction_signal = Signal_Generator.Update(row.batch_id, row.close, true);
                        direction = std::get<0>(direction_signal);
                        signal = std::get<1>(direction_signal);
                        continue;
                    }

                    if (row.high < exit_price) {
                        direction_signal = Signal_Generator.Update(row.batch_id, row.close, false);
                        direction = std::get<0>(direction_signal);
                        signal = std::get<1>(direction_signal);
                        continue;
                    }
                }

                if (row.high >= exit_price) {
                    call_value = bs.bs_call(exit_price, call_strike, exit_time / 252.0f, 0.07f, vol);
                    put_value = bs.bs_put(exit_price, put_strike, exit_time / 252.0f, 0.07f, vol);
                    summary.capital += call_pos * call_value + put_pos * put_value;
                    summary.pnl += call_pos * (call_value - call_price) + put_pos * (put_value - put_price);
                    position_flag = 0;
                    call_pos = 0.0f;
                    put_pos = 0.0f;
                    direction_signal = Signal_Generator.Update(row.batch_id, row.close, true);
                    direction = std::get<0>(direction_signal);
                    signal = std::get<1>(direction_signal);
                    continue;
                }
            } else if (position_flag < 0) {
                exit_time = trade_time - (row.batch_id - trade_batch) / 375.0f;

                if (row.high >= hedge_price) {
                    if (call_pos != 0.0f) {
                        call_value = bs.bs_call(hedge_price, call_strike, exit_time / 252.0f, 0.07f, vol);

                        if (strategy_params.strategy_type == 2 && call_pos != 0.0f) {
                            put_value = bs.bs_put(hedge_price, put_strike, exit_time / 252.0f, 0.07f, vol);
                            put_price = (put_price * put_pos + call_pos * call_value) /
                                        (put_pos + (call_pos * call_value / put_value));
                            put_pos += call_pos * call_value / put_value;
                        } else { summary.capital += call_pos * call_value; }

                        summary.pnl += call_pos * (call_value - call_price);
                        call_pos = 0.0f;
                    } else { call_value = 0.0f; }

                    if (row.high >= stop_loss) {
                        put_value = bs.bs_put(stop_loss, put_strike, exit_time / 252.0f, 0.07f, vol);
                        summary.capital += put_pos * put_value + call_pos * call_value;
                        summary.pnl += put_pos * (put_value - put_price) + call_pos * (call_value - call_price);
                        position_flag = 0;
                        put_pos = 0.0f;
                        call_pos = 0.0f;

                        if (summary.drawdown_count <= 1) {
                            if (summary.drawdown_count == 0) { summary.first_drawdown_pnl = summary.pnl; }
                            summary.second_drawdown_pnl = summary.pnl;
                        }

                        summary.drawdown_count++;
                        direction_signal = Signal_Generator.Update(row.batch_id, row.close, true);
                        direction = std::get<0>(direction_signal);
                        signal = std::get<1>(direction_signal);
                        continue;
                    }

                    if (row.low > exit_price) {
                        direction_signal = Signal_Generator.Update(row.batch_id, row.close, false);
                        direction = std::get<0>(direction_signal);
                        signal = std::get<1>(direction_signal);
                        continue;
                    }
                }

                if (row.low <= exit_price) {
                    put_value = bs.bs_put(exit_price, put_strike, exit_time / 252.0f, 0.07f, vol);
                    call_value = bs.bs_call(exit_price, call_strike, exit_time / 252.0f, 0.07f, vol);
                    summary.capital += put_pos * put_value + call_pos * call_value;
                    summary.pnl += put_pos * (put_value - put_price) + call_pos * (call_value - call_price);
                    position_flag = 0;
                    put_pos = 0.0f;
                    call_pos = 0.0f;
                    direction_signal = Signal_Generator.Update(row.batch_id, row.close, true);
                    direction = std::get<0>(direction_signal);
                    signal = std::get<1>(direction_signal);
                    continue;
                }
            }
            direction_signal = Signal_Generator.Update(row.batch_id, row.close, false);
            direction = std::get<0>(direction_signal);
            signal = std::get<1>(direction_signal);
        }
    }
    catch (const std::exception &e) {
        std::cout << std::endl << e.what();
    }
    if (summary.drawdown_count == 0) { summary.first_drawdown_pnl = summary.second_drawdown_pnl = summary.pnl; }
    return summary;
}

#endif
