
#include "stdafx.h"
#include "util.h"
#include "wordmgr.h"


wordmgr::wordmgr() {
    _next_word_idx = 0;
    _frozen = false;
}
wordmgr::~wordmgr() {
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

void wordmgr::restore_from(const wchar_t *path) {
#ifdef _DEBUG
    if (this->is_frozen())
        throw std::runtime_error(u8"wordmgr: append: object is frozen");
#endif

    _src_dict.clear();
    _src_word_dict.clear();
    _src_word_freq.clear();
    _src_words.clear();

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
}
void wordmgr::append(const char *word) {
#ifdef _DEBUG
    if (this->is_frozen())
        throw std::runtime_error(u8"wordmgr: append: object is frozen");
#endif

    std::string str(word);
    auto it = _src_dict.find(str);
    if (it == _src_dict.end()) {
        int word_idx = _next_word_idx++;
        _src_words.push_back(word_idx);
        _src_word_freq.push_back(1);
        _src_word_dict.push_back(str);
        _src_dict.insert(std::make_pair(str, word_idx));
    }
    else {
        int word_idx = it->second;
        _src_words.push_back(word_idx);
        _src_word_freq[word_idx]++;
    }
}

void wordmgr::freeze() {
    for (int i = 0; i < word_count(); i++) {
        long long pwr_3_4 = (long long)pow(_src_word_freq[i], 0.75);
        for (int j = 0; j < _src_word_freq[i]; j++) {
            _src_negative_unigram_3_4.push_back(i);
        }
    }
    _frozen = true;
}

bool wordmgr::is_frozen() const {
    return _frozen;
}

long long wordmgr::word_count() {
    return _src_dict.size();
}

long long wordmgr::corpus_word_count() {
    return _src_words.size();
}

int wordmgr::word_to_index(const char *word) const {
#ifdef _DEBUG
    if (!this->is_frozen())
        throw std::runtime_error(u8"wordmgr: word_to_index: object is not frozen");
#endif
    std::string str(word);
    auto it = _src_dict.find(str);
    if (it == _src_dict.end()) {
        return -1;
    }
    else {
        int word_idx = it->second;
        return word_idx;
    }
}

std::string wordmgr::index_to_word(int idx) const {
    if (idx >= 0 && idx < _src_word_dict.size()) {
        return _src_word_dict[idx];
    }
    throw std::runtime_error(u8"wordmgr: index_to_word: idx out of range");
}

void wordmgr::subsample_new() {
    _src_words_subsampled.clear();

    long long stored_word_count = _src_words.size();
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
            _src_words_subsampled.push_back(word_idx);
        }
    }
}
long long wordmgr::subsampled_corpus_word_count() {
    return _src_words_subsampled.size();
}
