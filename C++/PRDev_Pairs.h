//
// Created by Praneet Shaw on 4/2/23.
//

#ifndef TEST_PRDEV_PAIRS_H
#define TEST_PRDEV_PAIRS_H

#include "Sorting_Pairs.h"

class PRDev_Pairs : public Sorting_Pairs {

public:
    PRDev_Pairs(const int &, const int &, const float &, const float &, const float &, const float &, const float &,
                const float &, const float &, const float &, const float &, const float &);

    PRDev_Pairs(const Sorting_Pairs &other);

    PRDev_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const PRDev_Pairs &) const;

};


#endif //TEST_PRDEV_PAIRS_H
