
#include "stdafx.h"

#if 0
#include "wordmgr.h"


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


std::map<std::string, int> _src_dict;
std::vector<std::string> _src_word_dict;
std::vector<int> _src_word_freq;
std::vector<int> _src_words;

long long tick64() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}

static void do_it(const wchar_t *path) {
    // step 1: word dictionary
    std::wstring dict_path = std::wstring(path) + L".dict";
    FILE *fp = _wfopen(dict_path.c_str(), L"rb");
    DICT_FILE_HDR hdr;
    fread(&hdr, 1, sizeof(DICT_FILE_HDR), fp);
    int dw_sz = sizeof(DICTWORD) - 1 + hdr.word_len_max;
    DICTWORD *dw = (DICTWORD *)malloc(dw_sz);
    for (int i = 0; i < hdr.count; i++) {
        fread(dw, 1, dw_sz, fp);
        std::string word(dw->word, dw->word_len);
        _src_word_dict.push_back(word);
        _src_dict.insert(std::make_pair(word, i));
    }
    delete dw;
    fclose(fp);

    // step 2: file
    std::wstring corpus_path = std::wstring(path) + L".corpus";
    fp = _wfopen(corpus_path.c_str(), L"rb");
    long long stored_word_count;
    fread(&stored_word_count, 1, sizeof(stored_word_count), fp);
    _src_words.resize(stored_word_count);
    fread(&_src_words[0], 4, stored_word_count, fp);
    fclose(fp);

    // step 3: frequency
    _src_word_freq.resize(hdr.count, 0);
    for (int i = 0; i < stored_word_count; i++) {
        int word_idx = _src_words[i];
        _src_word_freq[word_idx]++;
    }

    // step 4: save
    std::wstring subs_corpus_path = std::wstring(path) + L".subs.corpus2";
    fp = _wfopen(subs_corpus_path.c_str(), L"wb");
    long long new_word_count = 0;
    fwrite(&new_word_count, 8, 1, fp);

    std::mt19937::result_type seed = (unsigned int)tick64();
    double t = 0.00001;
    long long rand_max = 1 << 30;
    auto rand = std::bind(std::uniform_int_distribution<long long>(0, rand_max - 1), std::mt19937(seed));

    int ccbuf = 10000000;
    int *buf = new int[ccbuf];
    int *ptr = buf;
    for (int i = 0; i < stored_word_count; i++) {
        int word_idx = _src_words[i];
        double f = _src_word_freq[word_idx] / (double)stored_word_count;
        double keep_prob = sqrt(t / f) + t / f;
        long long rand_val = rand();
        bool keep = rand_val < (rand_max * keep_prob);
        if (t / f >= 0.38254554) keep = true;
        if (keep) {
            new_word_count++;
            *ptr++ = word_idx;
            //fwrite(&word_idx, 4, 1, fp);
            if (ptr - buf == ccbuf) {
                fwrite(buf, 4, ccbuf, fp);
                ptr = buf;
            }
        }
    }
    fwrite(buf, 4, ptr - buf, fp);
    fseek(fp, 0, SEEK_SET);
    fwrite(&new_word_count, 8, 1, fp);
    fclose(fp);
}

int main() {
    do_it(L"D:\\Dev\\School\\word2vec_data\\wordset_large\\news.en-000.summary.v3");
    //do_it(L"D:\\Dev\\School\\word2vec_data\\text8.bin");
    return 0;
}
#endif