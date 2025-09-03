//
// Created by Praneet Shaw on 4/3/23.
//

#ifndef TEST_PRDIFF_PAIRS_H
#define TEST_PRDIFF_PAIRS_H

#include "Sorting_Pairs.h"

class PRDiff_Pairs : public Sorting_Pairs {

public:
    PRDiff_Pairs(const int &, const int &, const float &, const float &, const float &, const float &, const float &,
                 const float &, const float &, const float &, const float &, const float &);

    PRDiff_Pairs(const Sorting_Pairs &other);

    PRDiff_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const PRDiff_Pairs &) const;

};


#endif //TEST_PRDIFF_PAIRS_H
