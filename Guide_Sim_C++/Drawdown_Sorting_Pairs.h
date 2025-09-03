//
// Created by Praneet Shaw on 4/1/23.
//

#ifndef TEST_DRAWDOWN_SORTING_PAIRS_H
#define TEST_DRAWDOWN_SORTING_PAIRS_H

#include "Sorting_Pairs.h"

class Drawdown_Sorting_Pairs : public Sorting_Pairs {

public:
    Drawdown_Sorting_Pairs(const int &, const int &, const float &, const float &, const float &, const float &,
                           const float &,
                           const float &, const float &, const float &, const float &, const float &);

    Drawdown_Sorting_Pairs(const Sorting_Pairs &other);


    Drawdown_Sorting_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const Drawdown_Sorting_Pairs &) const;//const{ return A < rhs.A; }


};


#endif //TEST_DRAWDOWN_SORTING_PAIRS_H
