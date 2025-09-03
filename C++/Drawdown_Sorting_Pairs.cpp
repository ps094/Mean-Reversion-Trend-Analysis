//
// Created by Praneet Shaw on 4/1/23.
//
#ifndef TEST_DRAWDOWN_SORTING_PAIRS_CPP
#define TEST_DRAWDOWN_SORTING_PAIRS_CPP

#include "Drawdown_Sorting_Pairs.h"
#include<iostream>

Drawdown_Sorting_Pairs::Drawdown_Sorting_Pairs(const int &pos, const int &d_count, const float &profit,
                                               const float &TS1,
                                               const float &TS2, const float &TS3, const float &PDev, const float &MDev,
                                               const float &PRDev, const float &p_diff, const float &m_diff,
                                               const float &pr_diff) : Sorting_Pairs(pos, d_count, profit, TS1, TS2,
                                                                                     TS3, PDev,
                                                                                     MDev, PRDev, p_diff, m_diff,
                                                                                     pr_diff) {}

Drawdown_Sorting_Pairs::Drawdown_Sorting_Pairs(const Sorting_Pairs &other) : Sorting_Pairs(other) {}

Drawdown_Sorting_Pairs &Drawdown_Sorting_Pairs::operator=(const Sorting_Pairs &other) {
    Sorting_Pairs::operator=(other);
    return *this;
}

bool Drawdown_Sorting_Pairs::operator<(const Drawdown_Sorting_Pairs &rhs) const {
    return drawdown_count < rhs.drawdown_count;
}

#endif
