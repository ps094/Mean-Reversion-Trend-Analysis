//
// Created by Praneet Shaw on 4/3/23.
//
#ifndef TEST_PRDIFF_PAIRS_CPP
#define TEST_PRDIFF_PAIRS_CPP

#include "PRDiff_Pairs.h"

PRDiff_Pairs::PRDiff_Pairs(const int &pos, const int &d_count, const float &profit, const float &TS1,
                           const float &TS2, const float &TS3, const float &PDev, const float &MDev,
                           const float &PRDev, const float &p_diff, const float &m_diff,
                           const float &pr_diff) : Sorting_Pairs(pos, d_count, profit, TS1, TS2, TS3, PDev,
                                                                 MDev, PRDev, p_diff, m_diff, pr_diff) {}

PRDiff_Pairs::PRDiff_Pairs(const Sorting_Pairs &other) : Sorting_Pairs(other) {}

PRDiff_Pairs &PRDiff_Pairs::operator=(const Sorting_Pairs &other) {
    Sorting_Pairs::operator=(other);
    return *this;
}

bool PRDiff_Pairs::operator<(const PRDiff_Pairs &rhs) const { return prof_diff < rhs.prof_diff; }

#endif