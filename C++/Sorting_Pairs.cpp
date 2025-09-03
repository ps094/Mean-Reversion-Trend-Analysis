//
// Created by Praneet Shaw on 4/1/23.
//
#ifndef TEST_SORTING_PAIRS_CPP
#define TEST_SORTING_PAIRS_CPP

#include "Sorting_Pairs.h"


Sorting_Pairs::Sorting_Pairs(const int &pos, const int &d_count, const float &profit, const float &TS1,
                             const float &TS2, const float &TS3, const float &PDev, const float &MDev,
                             const float &PRDev, const float &p_diff, const float &m_diff, const float &pr_diff) :
        position(pos), drawdown_count(d_count), pnl(profit), taylor_sensitivity_1(TS1),
        taylor_sensitivity_2(TS2), taylor_sensitivity_3(TS3), pos_dev(PDev), mov_dev(MDev),
        prof_dev(PRDev), pos_diff(p_diff), mov_diff(m_diff), prof_diff(pr_diff) {}

Sorting_Pairs::Sorting_Pairs(const Sorting_Pairs &other) :
        position(other.position), drawdown_count(other.drawdown_count), pnl(other.pnl),
        taylor_sensitivity_1(other.taylor_sensitivity_1), taylor_sensitivity_2(other.taylor_sensitivity_2),
        taylor_sensitivity_3(other.taylor_sensitivity_3), pos_dev(other.pos_dev), mov_dev(other.mov_dev),
        prof_dev(other.prof_dev), pos_diff(other.pos_diff), mov_diff(other.mov_diff), prof_diff(other.prof_diff) {}

Sorting_Pairs &Sorting_Pairs::operator=(const Sorting_Pairs &other) {
    position = other.position;
    drawdown_count = other.drawdown_count;
    pnl = other.pnl;
    taylor_sensitivity_1 = other.taylor_sensitivity_1;
    taylor_sensitivity_2 = other.taylor_sensitivity_2;
    taylor_sensitivity_3 = other.taylor_sensitivity_3;
    pos_dev = other.pos_dev;
    mov_dev = other.mov_dev;
    prof_dev = other.prof_dev;
    pos_diff = other.pos_diff;
    mov_diff = other.mov_diff;
    prof_diff = other.prof_diff;
    return *this;
}


#endif
