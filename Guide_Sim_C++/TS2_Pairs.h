//
// Created by Praneet Shaw on 4/2/23.
//

#ifndef TEST_TS2_PAIRS_H
#define TEST_TS2_PAIRS_H

#include "Sorting_Pairs.h"

class TS2_Pairs : public Sorting_Pairs {

public:
    TS2_Pairs(const int &, const int &, const float &, const float &, const float &, const float &, const float &,
              const float &, const float &, const float &, const float &, const float &);

    TS2_Pairs(const Sorting_Pairs &other);

    TS2_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const TS2_Pairs &) const;

};


#endif //TEST_TS2_PAIRS_H
