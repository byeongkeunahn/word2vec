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

void add_batch_naive(float *dst, float *src, long long count) {
    for (long long i = 0; i < count; i++) {
        *dst++ += *src++;
    }
}
void add_batch_sse(float *dst, float *src, long long count) {
    __m256 *dst256 = (__m256 *)dst;
    __m256 *src256 = (__m256 *)src;
    long long i;
    long long end = count & ~7LL;
    for (i = 0; i < end; i += 8) {
        *dst256 = _mm256_add_ps(*dst256, *src256);
        *src256++;
        *dst256++;
    }
    for (; i < count; i++) {
        dst[i] += src[i];
    }
}
void add_batch_sse_2(float *dst, float *src, long long count) {
    __m256 *dst256 = (__m256 *)dst;
    __m256 *src256 = (__m256 *)src;
    long long i;
    long long end = count / 8;
    for (i = 0; i < end; i ++) {
        dst256[i] = _mm256_add_ps(dst256[i], src256[i]);
    }
    i <<= 3;
    for (; i < count; i++) {
        dst[i] += src[i];
    }
}

void test1() {
    float aa[16], bb[16], cc[16];
    __m256 *a = (__m256 *)&aa[3];
    __m256 *b = (__m256 *)&bb[5];
    __m256 *c = (__m256 *)&cc[6];
    for (int i = 0; i < 8; i++) {
        a->m256_f32[i] = i / 4.0f;
        b->m256_f32[i] = i / 2.0f;
    }
    *c = _mm256_add_ps(*a, *b);
    for (int i = 0; i < 8; i++) {
        printf("a[%d]=%.2f, b[%d]=%.2f, c[%d]=%.2f\n",
            i, a->m256_f32[i], i, b->m256_f32[i], i, c->m256_f32[i]);
    }
}

void test2() {
    float a[300], b[300];
    //float bufa[1000];
    //float bufb[1000];
    //float *a = (float *)(((((UINT_PTR)bufa) + 31) / 32) * 32);
    //float *b = (float *)(((((UINT_PTR)bufb) + 31) / 32) * 32);
    for (int i = 0; i < 300; i++) {
        a[i] = i / 4.0f;
        b[i] = i / 2.0f;
    }
    long long naive_start = tick64();
    for (int i = 0; i < 5000000; i++) {
        add_batch_sse_2(b, a, 300);
    }
    long long naive_end = tick64();
    for (int i = 253; i < 265; i++) {
        printf("a[%d]=%.2f, b[%d]=%.2f\n",
            i, a[i], i, b[i]);
    }

    for (int i = 0; i < 300; i++) {
        a[i] = i / 4.0f;
        b[i] = i / 2.0f;
    }
    long long sse_start = tick64();
    for (int i = 0; i < 5000000; i++) {
        add_batch_naive(b, a, 300);
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
    test2();
    system("pause");
    return 0;
}
#endif