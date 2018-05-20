#include "stdafx.h"

#if 0
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <immintrin.h>
#include <emmintrin.h>


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

void fnmadd_batch_naive(float *dst, float *src, float mul, long long count) {
    for (long long i = 0; i < count; i++) {
        *dst++ -= (*src++) * mul;
    }
}
void fnmadd_batch_avx(float *dst, float *src, float mul, long long count) {
    __m256 *dst256 = (__m256 *)dst;
    __m256 *src256 = (__m256 *)src;
    //mul = -mul;
    __m256  mul256 = _mm256_broadcastss_ps(*(__m128 *)&mul);
    long long i;
    long long end = count / 8;
    for (i = 0; i < end; i ++) {
        //__m256 tmp = _mm256_mul_ps(*src256++, mul256);
        //*dst256 = _mm256_add_ps(*dst256, tmp);
        //*dst256++;
        dst256[i] = _mm256_fnmadd_ps(src256[i], mul256, dst256[i]);
    }
    i <<= 3;
    for (; i < count; i++) {
        dst[i] -= src[i] * mul;
    }
}

void test() {
    const float mul = 2.0f;
    //float a[300], b[300];
    float bufa[1000];
    float bufb[1000];
    float *a = (float *)(((((UINT_PTR)bufa) + 31) / 32) * 32);
    float *b = (float *)(((((UINT_PTR)bufb) + 31) / 32) * 32);
    for (int i = 0; i < 300; i++) {
        a[i] = i / 4.0f;
        b[i] = i / 2.0f;
    }
    long long naive_start = tick64();
    for (int i = 0; i < 500000; i++) {
        fnmadd_batch_avx(b, a, mul, 300);
    }
    long long naive_end = tick64();
    for (int i = 253; i < 265; i++) {
        printf("a[%d]=%.2f, b[%d]=%.2f\n",
            i, a[i], i, b[i]);
    }
    printf("\n");
    for (int i = 0; i < 300; i++) {
        a[i] = i / 4.0f;
        b[i] = i / 2.0f;
    }
    long long sse_start = tick64();
    for (int i = 0; i < 500000; i++) {
        fnmadd_batch_naive(b, a, mul, 300);
    }
    long long sse_end = tick64();
    for (int i = 253; i < 265; i++) {
        printf("a[%d]=%.2f, b[%d]=%.2f\n",
            i, a[i], i, b[i]);
    }
    long long naive_us = (naive_end - naive_start) * 1000000 / tickfreq();
    long long sse_us = (sse_end - sse_start) * 1000000 / tickfreq();
    printf("naive = %lld us, SSE = %lld us\n", naive_us, sse_us);
}

int main() {
    test();
    system("pause");
    return 0;
}
#endif