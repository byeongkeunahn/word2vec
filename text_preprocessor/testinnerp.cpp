#include "stdafx.h"

#if 1
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>


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
float innerp_naive(float *v1, float *v2, long long count) {
    float innerp = 0;
    for (long long i = 0; i < count; i++) {
        innerp += v1[i] * v2[i];
    }
    return innerp;
}
float innerp_naive_unroll(float *v1, float *v2, long long count) {
    float innerp1 = 0;
    float innerp2 = 0;
    float innerp3 = 0;
    float innerp4 = 0;
    long long i;
    long long end = count & ~3LL;
    for (i = 0; i < end; i += 4) {
        innerp1 += v1[i] * v2[i];
        innerp2 += v1[i + 1] * v2[i + 1];
        innerp3 += v1[i + 2] * v2[i + 2];
        innerp4 += v1[i + 3] * v2[i + 3];
    }
    for (; i < count; i++) {
        innerp1 += v1[i] * v2[i];
    }
    return innerp1 + innerp2 + innerp3 + innerp4;
}
float innerp_sse(float *v1, float *v2, long long count) {
    __m256 *vv1 = (__m256 *)v1;
    __m256 *vv2 = (__m256 *)v2;
    long long i;
    long long end = count / 8;
    __m256 p = _mm256_setzero_ps();
    for (i = 0; i < end; i++) {
        __m256 c = _mm256_dp_ps(vv1[i], vv2[i], 0x11);
        p = _mm256_add_ps(p, c);
    }
    i <<= 3;
    float innerp = p.m256_f32[0] + p.m256_f32[4];
    for (; i < count; i++) {
        innerp += v1[i] * v2[i];
    }
    return innerp;
}

void test() {
    const float mul = 2.0f;
    //float a[300], b[300];
    float bufa[1000];
    float bufb[1000];
    float *a = (float *)(((((UINT_PTR)bufa) + 31) / 32) * 32);
    float *b = (float *)(((((UINT_PTR)bufb) + 31) / 32) * 32);
    for (int i = 0; i < 300; i++) {
        a[i] = i / 1.0f;
        b[i] = i / 2.0f;
    }
    long long naive_start = tick64();
    float innerp = 0;
    for (int i = 0; i < 1000000000; i++) {
        innerp += innerp_naive(a, b, 300);
    }
    long long naive_end = tick64();
    printf("%f\n", innerp);
    for (int i = 0; i < 300; i++) {
        a[i] = i / 1.0f;
        b[i] = i / 2.0f;
    }
    long long sse_start = tick64();
    innerp = 0;
    for (int i = 0; i < 1000000000; i++) {
        innerp += innerp_sse(a, b, 300);
    }
    long long sse_end = tick64();
    printf("%f\n", innerp);
    long long naive_us = (naive_end - naive_start) * 1000000 / tickfreq();
    long long sse_us = (sse_end - sse_start) * 1000000 / tickfreq();
    printf("naive = %lld us, SSE = %lld us\n", naive_us, sse_us);
}

void test2() {
    __m256 a, b;
    for (int i = 0; i < 8; i++) {
        a.m256_f32[i] = i * 1.0f;
        b.m256_f32[i] = i * 2.0f;
    }
    __m256 c = _mm256_dp_ps(a, b, 0xff);
    for (int i = 0; i < 8; i++) {
        printf("a[%d]=%.2f, b[%d]=%.2f, c[%d]=%.2f\n",
            i, a.m256_f32[i], i, b.m256_f32[i], i, c.m256_f32[i]);
    }
}
int main() {

    test();
    system("pause");
    return 0;
}
#endif