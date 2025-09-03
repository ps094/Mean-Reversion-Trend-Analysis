//
// Created by Praneet Shaw on 4/2/23.
//
#ifndef TEST_MDEV_PAIRS_CPP
#define TEST_MDEV_PAIRS_CPP

#include "MDev_Pairs.h"

MDev_Pairs::MDev_Pairs(const int &pos, const int &d_count, const float &profit, const float &TS1,
                       const float &TS2, const float &TS3, const float &PDev, const float &MDev,
                       const float &PRDev, const float &p_diff, const float &m_diff,
                       const float &pr_diff) : Sorting_Pairs(pos, d_count, profit, TS1, TS2, TS3, PDev,
                                                             MDev, PRDev, p_diff, m_diff, pr_diff) {}

MDev_Pairs::MDev_Pairs(const Sorting_Pairs &other) : Sorting_Pairs(other) {}

MDev_Pairs &MDev_Pairs::operator=(const Sorting_Pairs &other) {
    Sorting_Pairs::operator=(other);
    return *this;
}

bool MDev_Pairs::operator<(const MDev_Pairs &rhs) const { return mov_dev > rhs.mov_dev; }

#endif