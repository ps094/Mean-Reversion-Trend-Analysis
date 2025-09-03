//
// Created by Praneet Shaw on 4/2/23.
//

#ifndef TEST_PDIFF_PAIRS_H
#define TEST_PDIFF_PAIRS_H

#include "Sorting_Pairs.h"

class PDiff_Pairs : public Sorting_Pairs {

public:
    PDiff_Pairs(const int &, const int &, const float &, const float &, const float &, const float &, const float &,
                const float &, const float &, const float &, const float &, const float &);

    PDiff_Pairs(const Sorting_Pairs &other);

    PDiff_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const PDiff_Pairs &) const;

};

#endif //TEST_PDIFF_PAIRS_H
