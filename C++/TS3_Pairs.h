//
// Created by Praneet Shaw on 4/2/23.
//

#ifndef TEST_TS3_PAIRS_H
#define TEST_TS3_PAIRS_H

#include "Sorting_Pairs.h"

class TS3_Pairs : public Sorting_Pairs {

public:
    TS3_Pairs(const int &, const int &, const float &, const float &, const float &, const float &, const float &,
              const float &, const float &, const float &, const float &, const float &);

    TS3_Pairs(const Sorting_Pairs &other);

    TS3_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const TS3_Pairs &) const;

};


#endif //TEST_TS3_PAIRS_H
