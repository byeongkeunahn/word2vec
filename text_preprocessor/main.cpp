// text_preprocessor.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "wordmgr.h"


char lowc(char c) {
    return (c >= 'A'&&c <= 'Z') ? (c - 'A' + 'a') : (c);
}
bool isdelim(char c) {
    return (c <= ' ') || (c == '-');
}

void build_corpus(wordmgr &wm, const wchar_t *path) {
    const int buf_size = 1 << 20;

    FILE *fp = _wfopen(path, L"r");
    char *buf = (char *)malloc(buf_size);
    std::vector<char> v;

    char *pos, *end;
    while (1) {
        size_t sz = fread(buf, 1, buf_size, fp);
        if (sz == 0) break;
        pos = buf;
        end = pos + sz;
        while (pos < end) {
            char c = lowc(*pos++);
            if (isdelim(c)) {
                if (v.size() > 0) {
                    v.push_back('\0');
                    wm.append(&v[0]);
                    v.clear();
                }
            }
            else {
                v.push_back((char)c);
            }
        }
    }
    if (v.size() > 0) {
        v.push_back('\0');
        wm.append(&v[0]);
        v.clear();
    }
    fclose(fp);
    free(buf);
}

void write_to_file(wordmgr &wm, const wchar_t *path) {
    std::wstring dst_path = std::wstring(path) + L".processed";
    FILE *fp = _wfopen(dst_path.c_str(), L"w");
    char space = ' ';
    const char *newline = "\r\n";
    for (int i = 0; i < wm._dst_words.size(); i++) {
        std::string &str = wm._dst_word_dict[wm._dst_words[i]];
        fwrite(str.c_str(), 1, str.size(), fp);
        if (i % 10 == 9) {
            fwrite(newline, 1, 2, fp);
        }
        else {
            fwrite(&space, 1, 1, fp);
        }
    }
    fclose(fp);
}
#pragma pack(1)
struct DICTWORD {
    DWORD     word_len;
    char      word[1];
};
struct DICT_FILE_HDR {
    DWORD     count;
    DWORD     word_len_max;
};
#pragma pack()

void write_to_file_bin(wordmgr &wm, const wchar_t *path) {
    // step 1: dict file
    std::wstring dict_path = std::wstring(path) + L".dict";
    FILE *fp = _wfopen(dict_path.c_str(), L"wb");
    DICT_FILE_HDR hdr;
    hdr.count = (int)wm._dst_word_dict.size();
    hdr.word_len_max = 0;
    for (int i = 0; i < hdr.count; i++) {
        hdr.word_len_max = std::max(hdr.word_len_max, (DWORD)wm._dst_word_dict[i].size());
    }
    fwrite(&hdr, 1, sizeof(hdr), fp);
    int dw_sz = sizeof(DICTWORD) - 1 + hdr.word_len_max;
    DICTWORD *dw = (DICTWORD *)malloc(dw_sz);
    for (int i = 0; i < hdr.count; i++) {
        dw->word_len = wm._dst_word_dict[i].size();
        memcpy(dw->word, wm._dst_word_dict[i].c_str(), dw->word_len);
        fwrite(dw, 1, dw_sz, fp);
    }
    delete dw;
    fclose(fp);

    // step 2: word corpus
    std::wstring corpus_path = std::wstring(path) + L".corpus";
    fp = _wfopen(corpus_path.c_str(), L"wb");
    long long word_count = wm._dst_words.size();
    fwrite(&word_count, 1, sizeof(word_count), fp);
    fwrite(&wm._dst_words[0], 4, word_count, fp);
    fclose(fp);
}

void do_it(const wchar_t *path) {
    wordmgr wm;
    build_corpus(wm, path);
    write_to_file(wm, path);
}

#if 0
int main() {

    wordmgr wm;
    build_corpus(wm, L"D:\\Dev\\School\\word2vec_data\\text8");
    wm.freeze();
    write_to_file_bin(wm, L"D:\\Dev\\School\\word2vec_data\\text8.bin");

    return 0;
}
#endif

#if 0
int main() {

    wordmgr wm;
    wchar_t buf[3];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (!i && !j) continue;
            buf[0] = L'0' + i;
            buf[1] = L'0' + j;
            std::wstring path = std::wstring(L"D:\\Dev\\School\\word2vec_data\\wordset_large\\news.en-000")
                + std::wstring(buf) + std::wstring(L"-of-00100");
            build_corpus(wm, path.c_str());
        }
    }
    wm.freeze();
    write_to_file_bin(wm, L"D:\\Dev\\School\\word2vec_data\\wordset_large\\news.en-000.summary.v3");

    return 0;
}

#endif

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

void fma_batch_naive(float *dst, float *src1, float *src2, long long count) {
    for (long long i = 0; i < count; i++) {
        *dst++ += (*src1++) * (*src2++);
    }
}
void fma_batch_avx2(float *dst, float *src1, float *src2, long long count) {
    __m256 *dst_256 = (__m256 *)dst;
    __m256 *src1_256 = (__m256 *)src1;
    __m256 *src2_256 = (__m256 *)src2;
    long long i;
    long long end = count & ~7LL;
    for (i = 0; i < end; i += 8) {
        *dst_256 = _mm256_fmadd_ps(*src1_256++, *src2_256++, *dst_256);
        *dst_256++;
    }
    for (; i < count; i++) {
        dst[i] += src1[i] * src2[i];
    }
}

void test2() {
    float a[300], b[300], c[300];
    for (int i = 0; i < 300; i++) {
        a[i] = i / 1.0f;
        b[i] = i / 0.5f;
        c[i] = i / 0.25f;
    }
    long long naive_start = tick64();
    for (int i = 0; i < 5000; i++) {
        fma_batch_naive(c, a, b, 300);
    }
    long long naive_end = tick64();
    for (int i = 253; i < 265; i++) {
        printf("a[%d]=%.2f, b[%d]=%.2f, c[%d]=%.2f\n",
            i, a[i], i, b[i], i, c[i]);
    }
    printf("\n");
    for (int i = 0; i < 300; i++) {
        a[i] = i / 1.0f;
        b[i] = i / 0.5f;
        c[i] = i / 0.25f;
    }
    long long sse_start = tick64();
    for (int i = 0; i < 5000; i++) {
        fma_batch_avx2(c, a, b, 300);
    }
    long long sse_end = tick64();
    for (int i = 253; i < 265; i++) {
        printf("a[%d]=%.2f, b[%d]=%.2f, c[%d]=%.2f\n",
            i, a[i], i, b[i], i, c[i]);
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
