
#include "stdafx.h"
#include "util.h"
#include "wordmgr.h"


wordmgr::wordmgr() {
    _next_idx = 0;
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
        int word_idx = _next_idx++;
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

void wordmgr::train_gen_init(int neighbors) {
#ifdef _DEBUG
    if (!this->is_frozen())
        throw std::runtime_error(u8"wordmgr: train_gen_init: object is not frozen");
#endif
    _state.neighbors = neighbors;
    _state.next_center = 0;
    _state.next_offset = -neighbors;
}

long long wordmgr::train_gen(long long desired_count, word_io_pair *buf) {
#ifdef _DEBUG
    if (!this->is_frozen())
        throw std::runtime_error(u8"wordmgr: train_gen: object is not frozen");
#endif
    long long count = 0;
    while (count < desired_count) {
        // try adding current example
        int target_word = _state.next_center + _state.next_offset;
        if (target_word >= 0 && target_word < corpus_word_count()) {
            *buf++ = { _src_words[_state.next_center], _src_words[target_word] };
            count++;
        }
        // advance to next example
        _state.next_offset++;
        if (!_state.next_offset) {
            _state.next_offset++;
        }
        else if (_state.next_offset > _state.neighbors) {
            _state.next_center++;
            if (_state.next_center >= corpus_word_count()) {
                break;
            }
            _state.next_offset = -(_state.neighbors);
        }
    }

    return count;
}

long long wordmgr::train_gen_all(word_io_pair *buf) {
    _state_type _state_backup = _state;
    train_gen_init(_state.neighbors);
    long long gen_count = train_gen(1LL << 48, buf);
    _state = _state_backup;
    return gen_count;
}

long long wordmgr::train_gen_sgd_batch(word_io_pair * buf, int K, long long desired_count) {
    std::mt19937::result_type seed = (unsigned int)tick64();
    auto rand = std::bind(std::uniform_int_distribution<long long>(0, this->corpus_word_count() - 1), std::mt19937(seed));
    auto rand_window = std::bind(std::uniform_int_distribution<long long>(-K+1, K), std::mt19937(seed));
    for (int i = 0; i < desired_count; i++) {
        auto center_word = rand();
        auto offset = rand_window();
        if (offset <= 0) offset -= 1;
        if (center_word + offset < 0 || center_word + offset >= this->corpus_word_count()) {
            i--; continue;
        }
        *buf++ = { _src_words[center_word], _src_words[center_word + offset] };
    }
    return desired_count;
}
/*
long long wordmgr::train_gen_skip_gram(te_skip_gram *buf, int K, long long desired_count) {
    std::mt19937::result_type seed = (unsigned int)tick64();
    auto rand_window_sz = std::bind(std::uniform_int_distribution<int>(1, K), std::mt19937(seed));
    auto rand = std::bind(std::uniform_int_distribution<long long>(0, this->corpus_word_count() - 1), std::mt19937(seed));
    for (int i = 0; i < desired_count; i++) {
        auto out_window_sz = rand_window_sz();
        auto center_word = 


        auto center_word = rand();
        auto offset = rand_window();
        if (offset <= 0) offset -= 1;
        if (center_word + offset < 0 || center_word + offset >= this->corpus_word_count()) {
            i--; continue;
        }
        //*buf++ = { _src_words[center_word], _src_words[center_word + offset] };
    }
    return desired_count;
}
*/
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

