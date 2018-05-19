
#include "stdafx.h"
#include "util.h"

long long tickfreq() {
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    return li.QuadPart;
}

long long tick64() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}

float sigmoid(float x) {
    return 1 / (1 + exp(-x));
}

void softmax(const float *in, float *out, long long count) {
    float max_val = -INFINITY;
    for (long long i = 0; i < count; i++) {
        max_val = std::max(max_val, in[i]);
    }
    float cumul = 0;
    for (long long i = 0; i < count; i++) {
        float in_2 = in[i] - max_val;
        cumul += out[i] = exp(in_2);
    }
    for (long long i = 0; i < count; i++) {
        out[i] /= cumul;
    }
}
