//
// Created by Praneet Shaw on 4/2/23.
//

#ifndef TEST_PDEV_PAIRS_H
#define TEST_PDEV_PAIRS_H

#include "Sorting_Pairs.h"

class PDev_Pairs : public Sorting_Pairs {

public:
    PDev_Pairs(const int &, const int &, const float &, const float &, const float &, const float &, const float &,
               const float &, const float &, const float &, const float &, const float &);

    PDev_Pairs(const Sorting_Pairs &other);

    PDev_Pairs &operator=(const Sorting_Pairs &);

    bool operator<(const PDev_Pairs &) const;

};


#endif //TEST_PDEV_PAIRS_H
