//
// Created by Praneet Shaw on 4/3/23.
//
#ifndef TEST_MDIFF_PAIRS_CPP
#define TEST_MDIFF_PAIRS_CPP

#include "MDiff_Pairs.h"

MDiff_Pairs::MDiff_Pairs(const int &pos, const int &d_count, const float &profit, const float &TS1,
                         const float &TS2, const float &TS3, const float &PDev, const float &MDev,
                         const float &PRDev, const float &p_diff, const float &m_diff,
                         const float &pr_diff) : Sorting_Pairs(pos, d_count, profit, TS1, TS2, TS3, PDev,
                                                               MDev, PRDev, p_diff, m_diff, pr_diff) {}

MDiff_Pairs::MDiff_Pairs(const Sorting_Pairs &other) : Sorting_Pairs(other) {}

MDiff_Pairs &MDiff_Pairs::operator=(const Sorting_Pairs &other) {
    Sorting_Pairs::operator=(other);
    return *this;
}

bool MDiff_Pairs::operator<(const MDiff_Pairs &rhs) const { return mov_diff < rhs.mov_diff; }

#endif