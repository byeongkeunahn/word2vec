
#include "stdafx.h"
#include "wordmgr.h"


wordmgr::wordmgr() {
    _next_idx = 0;
    _frozen = false;
}
wordmgr::~wordmgr() {
}

void wordmgr::append(const char *word) {
#ifdef _DEBUG
    if (this->is_frozen())
        throw std::runtime_error(u8"wordmgr: append: object is frozen");
#endif

    std::string str(word);
    auto it = _dict.find(str);
    if (it == _dict.end()) {
        int word_idx = _next_idx++;
        _words.push_back(word_idx);
        _word_freq.push_back(1);
        _dict.insert(std::make_pair(str, word_idx));
    }
    else {
        int word_idx = it->second;
        _words.push_back(word_idx);
        _word_freq[word_idx]++;
    }
}

void wordmgr::freeze() {
    _frozen = true;
}

bool wordmgr::is_frozen() {
    return _frozen;
}

long long wordmgr::word_count() {
    return _dict.size();
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

long long wordmgr::train_gen(long long desired_count, long long *in, long long *out) {
#ifdef _DEBUG
    if (!this->is_frozen())
        throw std::runtime_error(u8"wordmgr: train_gen: object is not frozen");
#endif
    long long count = 0;
    while (count < desired_count) {
        // try adding current example
        int target_word = _state.next_center + _state.next_offset;
        if (target_word >= 0 && target_word < word_count()) {
            *in++ = _state.next_center;
            *out++ = target_word;
            count++;
        }
        // advance to next example
        _state.next_offset++;
        if (!_state.next_offset) {
            _state.next_offset++;
        }
        else if (_state.next_offset > _state.neighbors) {
            _state.next_center++;
            if (_state.next_center >= word_count()) {
                break;
            }
            _state.next_offset = -(_state.neighbors);
        }
    }

    return count;
}

