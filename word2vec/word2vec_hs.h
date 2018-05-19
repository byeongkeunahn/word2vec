
#pragma once

#include "wordmgr.h"

typedef enum {
    aio_none,
    aio_zero,
    aio_random
} alloc_init_options;

class word2vec_hs {
public:
    word2vec_hs();
    ~word2vec_hs();

public:
    void init(long long V, long long D, const int *freq_table);
    void load(const wchar_t *path);
    void save(const wchar_t *path);
    void destroy();

public:
    void set_learning_rate(float e);
    // SGD, mini-batch or full-batch depending on ccTrain supplied
    // indices should be 0-based
    // returns current value of the loss function
    float step(const word_io_pair *trains, long long ccTrain);
    float step_skip_gram(wordmgr &wm, const int *trains, long long ccTrain, int thread_id = -1);
    float step_skip_gram_sgd(wordmgr &wm, long long ccTrain);
    float step_momentum(const word_io_pair *trains, long long ccTrain);

public:
    // computes the word most similar to "word2 - word1 + word3"
    long long reasoning_task(long long word1, long long word2, long long word3);
    int reasoning_task_k(long long word1, long long word2, long long word3,
        int *similar_words, float *similarities, int k);
    int most_similar_k(int word, int *similar_words, float *similarities, int k);
    float similarity(float *vec1, float *vec2);

private:
    float **alloc_2d(long long dim1, long long dim2, alloc_init_options aio = aio_none);
    void free_2d(float **p);
    void fwrite_2d(float **p, long long dim1, long long dim2, FILE *fp);
    void fread_2d(float **p, long long dim1, long long dim2, FILE *fp);
    void fill_random(float *p, long long count);
    void add_batch(float *dst, float *src, long long count);
    void fmadd_batch(float *dst, float *src1, float *src2, long long count);

    unsigned long long random() {
        _rand_state = _rand_state * 0x5deece66dull + 0xb;
        return _rand_state;
    }
    unsigned long long _rand_state;

    void copy_W2(float *dst, long long idx) {
        std::lock_guard<std::mutex> lock(W2_mutices[idx]);
        memcpy(dst, _W2[idx], sizeof(float)*_D);
    }

private:
    bool _is_init;
    long long _V;
    long long _D;
    float _e;
    float _g; // gamma for momentum
    float **_W1; // [V][D]
    float **_W2; // [V-1][D]
    float **_W1_momentum;
    float **_W2_momentum;
    struct {
        float **W1_update;
        float **W2_update;
    } TLS[32];

    int *_node_tree;
    bool *_parent_side; // am I left- or right- attached to the parent? (left=true, right=false)

    std::mutex *W1_mutices;
    std::mutex *W2_mutices;
};

