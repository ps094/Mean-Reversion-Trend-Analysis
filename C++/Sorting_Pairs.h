//
// Created by Praneet Shaw on 4/1/23.
//

#ifndef TEST_SORTING_PAIRS_H
#define TEST_SORTING_PAIRS_H


class Sorting_Pairs {

public:
    int position, drawdown_count;
    float pnl, taylor_sensitivity_1, taylor_sensitivity_2, taylor_sensitivity_3, pos_dev, mov_dev, prof_dev, pos_diff,
            mov_diff, prof_diff;

    Sorting_Pairs(const int &, const int &, const float &, const float &, const float &, const float &, const float &,
                  const float &, const float &, const float &, const float &, const float &);

    Sorting_Pairs(const Sorting_Pairs &other);

    Sorting_Pairs &operator=(const Sorting_Pairs &);

};


#endif //TEST_SORTING_PAIRS_H
