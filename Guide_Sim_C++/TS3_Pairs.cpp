//
// Created by Praneet Shaw on 4/2/23.
//
#ifndef TEST_TS3_PAIRS_CPP
#define TEST_TS3_PAIRS_CPP

#include "TS3_Pairs.h"

TS3_Pairs::TS3_Pairs(const int &pos, const int &d_count, const float &profit, const float &TS1,
                     const float &TS2, const float &TS3, const float &PDev, const float &MDev,
                     const float &PRDev, const float &p_diff, const float &m_diff,
                     const float &pr_diff) : Sorting_Pairs(pos, d_count, profit, TS1, TS2, TS3, PDev,
                                                           MDev, PRDev, p_diff, m_diff, pr_diff) {}

TS3_Pairs::TS3_Pairs(const Sorting_Pairs &other) : Sorting_Pairs(other) {}

TS3_Pairs &TS3_Pairs::operator=(const Sorting_Pairs &other) {
    Sorting_Pairs::operator=(other);
    return *this;
}

bool TS3_Pairs::operator<(const TS3_Pairs &rhs) const { return taylor_sensitivity_3 < rhs.taylor_sensitivity_3; }

#endif