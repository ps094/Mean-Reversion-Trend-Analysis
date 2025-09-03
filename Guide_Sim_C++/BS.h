#ifndef BS_H
#define BS_H

#include<cmath>

class BS {
public:
    [[nodiscard]] static inline float cdf1(const float &x) {
        return (1.0f - (1.0f / (powf(2.0f * M_PI, 0.5f))) * expf(-0.5f * x * x) *
                       ((1.0f / (1.0f + 0.2316419f * x)) * (0.319381530f +
                                                            (1.0f / (1.0f +
                                                                     0.2316419f *
                                                                     x)) *
                                                            (-0.356563782f +
                                                             (1.0f /
                                                              (1.0f +
                                                               0.2316419f *
                                                               x)) *
                                                             (1.781477937f +
                                                              (1.0f /
                                                               (1.0f +
                                                                0.2316419f *
                                                                x)) *
                                                              (-1.821255978f +
                                                               1.330274429f *
                                                               (1.0f /
                                                                (1.0f +
                                                                 0.2316419f *
                                                                 x))))))));
    }

    [[nodiscard]] static inline float cdf2(const float &x) {
        return 1.0f -
               (1.0f - (1.0f / (powf(2 * M_PI, 0.5f))) * expf(-0.5f * x * x) * ((1.0f / (1.0f - 0.2316419f * x)) *
                                                                                (0.319381530f + (1.0f / (1.0f -
                                                                                                         0.2316419f *
                                                                                                         x)) *
                                                                                                (-0.356563782f +
                                                                                                 (1.0f / (1.0f -
                                                                                                          0.2316419f *
                                                                                                          x)) *
                                                                                                 (1.781477937f +
                                                                                                  (1.0f / (1.0f -
                                                                                                           0.2316419f *
                                                                                                           x)) *
                                                                                                  (-1.821255978f +
                                                                                                   1.330274429f *
                                                                                                   (1.0f /
                                                                                                    (1.0f -
                                                                                                     0.2316419f *
                                                                                                     x))))))));
    }

    [[nodiscard]] static inline float cdf(const float &x) { return x >= 0.0f ? cdf1(x) : cdf2(x); }

    [[nodiscard]] static inline float
    d1(const float &S, const float &K, const float &T, const float &r, const float &sigma) {
        return (logf(S / K) + (r + powf(sigma, 2.0f) / 2.0f) * T) / (sigma * sqrtf(T));
    }

    [[nodiscard]] static inline float
    d2(const float &S, const float &K, const float &T, const float &r, const float &sigma) {
        return d1(S, K, T, r, sigma) - sigma * sqrtf(T);
    }

    [[nodiscard]] static inline float
    bs_call(const float &S, const float &K, const float &T, const float &r, const float &sigma) {
        return S * cdf(d1(S, K, T, r, sigma)) - K * expf(-r * T) * cdf(d2(S, K, T, r, sigma));
    }

    [[nodiscard]] static inline float
    bs_put(const float &S, const float &K, const float &T, const float &r, const float &sigma) {
        return K * expf(-r * T) - S + bs_call(S, K, T, r, sigma);
    }
};

#endif