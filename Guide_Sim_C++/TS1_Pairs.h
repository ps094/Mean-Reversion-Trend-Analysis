//
// Created by Praneet Shaw on 4/2/23.
//

#ifndef TEST_TS1_PAIRS_H
#define TEST_TS1_PAIRS_H

#include "Sorting_Pairs.h"

class TS1_Pairs : public Sorting_Pairs {

public:
    TS1_Pairs(const int &, const int &, const float &, const float &, const float &, const float &, const float &,
              const float &, const float &, const float &, const float &, const float &);

    TS1_Pairs(const Sorting_Pairs &other);

    TS1_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const TS1_Pairs &) const;
};


#endif //TEST_TS1_PAIRS_H
