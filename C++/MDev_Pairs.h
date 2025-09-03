//
// Created by Praneet Shaw on 4/2/23.
//

#ifndef TEST_MDEV_PAIRS_H
#define TEST_MDEV_PAIRS_H

#include "Sorting_Pairs.h"

class MDev_Pairs : public Sorting_Pairs {

public:
    MDev_Pairs(const int &, const int &, const float &, const float &, const float &, const float &, const float &,
               const float &, const float &, const float &, const float &, const float &);

    MDev_Pairs(const Sorting_Pairs &other);

    MDev_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const MDev_Pairs &) const;
};


#endif //TEST_MDEV_PAIRS_H
