//
// Created by Praneet Shaw on 4/3/23.
//

#ifndef TEST_MDIFF_PAIRS_H
#define TEST_MDIFF_PAIRS_H

#include "Sorting_Pairs.h"

class MDiff_Pairs : public Sorting_Pairs {

public:
    MDiff_Pairs(const int &, const int &, const float &, const float &, const float &, const float &, const float &,
                const float &, const float &, const float &, const float &, const float &);

    MDiff_Pairs(const Sorting_Pairs &other);

    MDiff_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const MDiff_Pairs &) const;

};


#endif //TEST_MDIFF_PAIRS_H
