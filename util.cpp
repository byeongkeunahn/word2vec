
#include "stdafx.h"

void softmax(const double *in, double *out, long long count) {
    double max_val = -INFINITY;
    for (long long i = 0; i < count; i++) {
        max_val = std::max(max_val, in[i]);
    }
    double cumul = 0;
    for (long long i = 0; i < count; i++) {
        double in_2 = in[i] - max_val;
        cumul += out[i] = exp(in_2);
    }
    for (long long i = 0; i < count; i++) {
        out[i] /= cumul;
    }
}
