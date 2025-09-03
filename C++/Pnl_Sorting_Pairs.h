//
// Created by Praneet Shaw on 4/1/23.
//

#ifndef TEST_PNL_SORTING_PAIRS_H
#define TEST_PNL_SORTING_PAIRS_H

#include "Sorting_Pairs.h"

class Pnl_Sorting_Pairs : public Sorting_Pairs {

public:
    Pnl_Sorting_Pairs(const int &, const int &, const float &, const float &, const float &, const float &,
                      const float &,
                      const float &, const float &, const float &, const float &, const float &);

    Pnl_Sorting_Pairs(const Sorting_Pairs &other);

    Pnl_Sorting_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const Pnl_Sorting_Pairs &) const;
};


#endif //TEST_PNL_SORTING_PAIRS_H
