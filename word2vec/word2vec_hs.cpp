
#include "stdafx.h"
#include "util.h"
#include "heap.h"
#include "word2vec_hs.h"


word2vec_hs::word2vec_hs() {
    _is_init = false;
    for (int i = 0; i < 32; i++) {
        TLS[i].W1_update = nullptr;
        TLS[i].W2_update = nullptr;
    }
}

word2vec_hs::~word2vec_hs() {
    destroy();
    for (int i = 0; i < 32; i++) {
        if (TLS[i].W1_update != nullptr) {
            free_2d(TLS[i].W1_update);
        }
        if (TLS[i].W2_update != nullptr) {
            free_2d(TLS[i].W2_update);
        }
    }
}

void word2vec_hs::init(long long V, long long D, const int *freq_table) {
    if (_is_init) destroy();
    _V = V;
    _D = D;
    _W1 = alloc_2d(V, D, aio_random);
    _W2 = alloc_2d(V - 1, D, aio_random);
    _e = 0.01f;

    W1_mutices = new std::mutex[_V]();
    W2_mutices = new std::mutex[_V - 1]();

    // build huffman tree
    _node_tree = new int[2 * V - 1];
    _parent_side = new bool[2 * V - 2];
    heap h(V);
    for (int i = 0; i < V; i++) {
        h.add({ i, freq_table[i] });
    }
    for (int i = V; i < 2 * V - 1; i++) {
        heap_data top1 = h.extract_min();
        heap_data top2 = h.extract_min();
        //printf("%d %d\n", top1.id, top2.id);
        _node_tree[top1.id] = _node_tree[top2.id] = i;
        _parent_side[top1.id] = true;
        _parent_side[top2.id] = false;
        h.add({ i, top1.num + top2.num });
    }
    _node_tree[2 * V - 2] = -1; // indicates root of the tree
    // linearize huffman tree (TODO, if necessary)

    _is_init = true;
}

void word2vec_hs::load(const wchar_t *path) {
    FILE *fp = _wfopen(path, L"rb");
    if (fp) {
        fread_2d(_W1, _V, _D, fp);
        fread_2d(_W2, _V - 1, _D, fp);
        fclose(fp);
        _wprintf_p(L"Loading parameters from %s\r\n", path);
    }
    else {
        _wprintf_p(L"Failed to load parameters from %s; the model will be trained anew\r\n", path);
    }
}

void word2vec_hs::save(const wchar_t *path) {
    std::wostringstream stringStream;
    stringStream << path << L".backup." << time(NULL);
    _wrename(path, stringStream.str().c_str());
    if (!_is_init) throw std::runtime_error(u8"word2vec_hs: cannot save as instance not initialized");
    FILE *fp = _wfopen(path, L"wb");
    fwrite_2d(_W1, _V, _D, fp);
    fwrite_2d(_W2, _V - 1, _D, fp);
    fclose(fp);
}

void word2vec_hs::destroy() {
    if (_is_init) {
        free_2d(_W1);
        free_2d(_W2);
        delete[] _node_tree;
        delete[] _parent_side;
        _is_init = false;
    }
}

void word2vec_hs::set_learning_rate(float e) {
    _e = e;
}

float word2vec_hs::step_skip_gram(wordmgr &wm, const int *trains, long long ccTrain, int thread_id) {
    if (!_is_init) {
        throw std::runtime_error(u8"word2vec_hs: step() cannot be called before init");
    }
    float *hidden_layer = new float[_D];
    float *W1_update = new float[_D];
    float *W2_local = new float[_D];
    bool *W2_dirty = new bool[_V - 1]();

    float **W2_update;
    if (thread_id != -1) {
        W2_update = TLS[thread_id].W2_update;
        if (W2_update == nullptr) {
            W2_update = alloc_2d(_V - 1, _D, aio_none);
            TLS[thread_id].W2_update = W2_update;
        }
    }
    else {
        W2_update = alloc_2d(_V - 1, _D, aio_none);
    }
    float loss = 0;

    for (int _i = 0; _i < ccTrain; _i++) {
        // training example generation
        long long in_idx = trains[_i];
        //long long in = wm._src_words[in_idx];
        long long in = wm._src_words_subsampled[in_idx];
        long long window_sz = 5;

        // forward propagation
        {
            std::lock_guard<std::mutex> lock(W1_mutices[in]);
            memcpy(hidden_layer, _W1[in], sizeof(float) * _D);
        }
        memset(W1_update, 0, sizeof(float) * _D);

        float single_loss = 0;
        long long out_from = std::max(0LL, in_idx - window_sz);
        long long out_to = std::min(wm.subsampled_corpus_word_count() - 1, in_idx + window_sz);
        for (long long out_idx = out_from; out_idx <= out_to; out_idx++) {
            if (out_idx == in_idx) continue;
            //long long out = wm._src_words[out_idx];
            long long out = wm._src_words_subsampled[out_idx];
            long long cur_node = _node_tree[out];
            bool side = _parent_side[out];
            do {
                long long outi = cur_node - _V;
                if (!W2_dirty[outi]) {
                    memset(W2_update[outi], 0, sizeof(float) * _D);
                }
                copy_W2(W2_local, outi);
                // forward propagation
                float innerp = 0;
                for (int j = 0; j < _D; j++) innerp += W2_local[j] * hidden_layer[j];
                float sign = side ? 1.0f : -1.0f;
                float a = sigmoid(sign * innerp);
                single_loss += -logf(a + 0.0000000000001f);
                float tmp = (-1) * _e * sign * (1 - a);
                // update W1 layer
                for (int j = 0; j < _D; j++) {
                    //W1_update[j] -= _e * sign * (-_W2[outi][j] * (1 - a));
                    W1_update[j] -= W2_local[j] * tmp;
                    W2_update[outi][j] -= hidden_layer[j] * tmp;
                }
                // update HS layer
                //for (int j = 0; j < _D; j++) W2_update[outi][j] -= _e * sign * (-hidden_layer[j] * (1 - a));
                //for (int j = 0; j < _D; j++) W2_update[outi][j] -= hidden_layer[j] * tmp;
                W2_dirty[outi] = true;
                side = _parent_side[cur_node];
                cur_node = _node_tree[cur_node];
            } while (cur_node != -1);
        }
        loss += single_loss / (out_to - out_from);
        std::lock_guard<std::mutex> lock(W1_mutices[in]);
        //for (int j = 0; j < _D; j++) {
        //    //_W1_momentum[in][j] = _g * _W1_momentum[in][j] + _e * W1_update[j];
        //    //_W1[in][j] += _W1_momentum[in][j];
        //    _W1[in][j] += W1_update[j];
        //}
        add_batch(_W1[in], W1_update, _D);
    }
    for (int i = 0; i < _V - 1; i++) {
        if (W2_dirty[i]) {
            std::lock_guard<std::mutex> lock(W2_mutices[i]);
            //for (int j = 0; j < _D; j++) {
            //    //_W2_momentum[i][j] = _g * (_W2_momentum[i][j]) + _e * (W2_update[i][j]);
            //    //_W2[i][j] += _W2_momentum[i][j];
            //    _W2[i][j] += W2_update[i][j];
            //}
            add_batch(_W2[i], W2_update[i], _D);
        }
    }
    delete[] hidden_layer;
    delete[] W1_update;
    delete[] W2_dirty;
    if (thread_id == -1) {
        free_2d(W2_update);
    }
    return loss / ccTrain;
}

float word2vec_hs::step_cbow(wordmgr &wm, const int *trains, long long ccTrain, int thread_id) {
    if (!_is_init) {
        throw std::runtime_error(u8"word2vec_hs: step() cannot be called before init");
    }
    float *hidden_layer = new float[_D];
    float *hidden_grad = new float[_D];
    float *W2_local = new float[_D];
    bool *W2_dirty = new bool[_V - 1]();

    float **W2_update;
    if (thread_id != -1) {
        W2_update = TLS[thread_id].W2_update;
        if (W2_update == nullptr) {
            W2_update = alloc_2d(_V - 1, _D, aio_none);
            TLS[thread_id].W2_update = W2_update;
        }
    }
    else {
        W2_update = alloc_2d(_V - 1, _D, aio_none);
    }
    float loss = 0;

    for (int _i = 0; _i < ccTrain; _i++) {
        long long out_idx = trains[_i];
        long long out = wm._src_words_subsampled[out_idx];
        long long window_sz = 5;

        // forward propagation 1: hidden layer
        long long in_from = std::max(0LL, out_idx - window_sz);
        long long in_to = std::min(wm.subsampled_corpus_word_count() - 1, out_idx + window_sz);
        memset(hidden_layer, 0, sizeof(float) * _D);
        for (long long in_idx = in_from; in_idx <= in_to; in_idx++) {
            if (out_idx == in_idx) continue;
            long long in = wm._src_words_subsampled[in_idx];
            std::lock_guard<std::mutex> lock(W1_mutices[in]);
            add_batch(hidden_layer, _W1[in], _D);
        }
        // forward propagation 2: final layer (loss function)
        memset(hidden_grad, 0, sizeof(float) * _D);
        long long cur_node = _node_tree[out];
        bool side = _parent_side[out];
        float single_loss = 0;
        do {
            long long outi = cur_node - _V;
            if (!W2_dirty[outi]) {
                memset(W2_update[outi], 0, sizeof(float) * _D);
            }
            copy_W2(W2_local, outi);
            // forward propagation
            float innerp = 0;
            for (int j = 0; j < _D; j++) innerp += W2_local[j] * hidden_layer[j];
            float sign = side ? 1.0f : -1.0f;
            float a = sigmoid(sign * innerp);
            single_loss += -logf(a + 0.0000000000001f);
            float tmp = (-1) * _e * sign * (1 - a);

            // backpropagation 1: HS layer and hidden layer
            for (int j = 0; j < _D; j++) {
                hidden_grad[j] -= W2_local[j] * tmp;
                W2_update[outi][j] -= hidden_layer[j] * tmp;
            }

            W2_dirty[outi] = true;
            side = _parent_side[cur_node];
            cur_node = _node_tree[cur_node];
        } while (cur_node != -1);
        
        // backpropagation 2: W1 layer
        for (long long in_idx = in_from; in_idx <= in_to; in_idx++) {
            if (out_idx == in_idx) continue;
            long long in = wm._src_words_subsampled[in_idx];
            std::lock_guard<std::mutex> lock(W1_mutices[in]);
            add_batch(_W1[in], hidden_grad, _D);
        }

        loss += single_loss;
    }
    for (int i = 0; i < _V - 1; i++) {
        if (W2_dirty[i]) {
            std::lock_guard<std::mutex> lock(W2_mutices[i]);
            add_batch(_W2[i], W2_update[i], _D);
        }
    }
    delete[] hidden_layer;
    delete[] hidden_grad;
    delete[] W2_local;
    delete[] W2_dirty;
    if (thread_id == -1) {
        free_2d(W2_update);
    }
    return loss / ccTrain;
}

long long word2vec_hs::reasoning_task(long long word1, long long word2, long long word3) {
    if (!_is_init)
        throw std::runtime_error(u8"word2vec_hs: reasoning_task: not initialized");

    float *vec = new float[_D]();
    for (int i = 0; i < _D; i++) {
        vec[i] += _W1[word2][i];
    }
    for (int i = 0; i < _D; i++) {
        vec[i] -= _W1[word1][i];
    }
    for (int i = 0; i < _D; i++) {
        vec[i] += _W1[word3][i];
    }
    float vec_norm = 0;
    for (int i = 0; i < _D; i++) {
        vec_norm += vec[i] * vec[i];
    }
    vec_norm = sqrt(vec_norm);

    float best_similarity = 0;
    long long best_word = -1;
    for (int v = 0; v < _V; v++) {
        if (v == word1 || v == word2 || v == word3) continue;
        float innerp = 0;
        for (int d = 0; d < _D; d++) {
            innerp += vec[d] * _W1[v][d];
        }
        float word_norm = 0;
        for (int d = 0; d < _D; d++) {
            word_norm += _W1[v][d] * _W1[v][d];
        }
        word_norm = sqrt(word_norm);
        float similarity = innerp / (vec_norm * word_norm);
        if (best_word == -1 || similarity > best_similarity) {
            best_similarity = similarity;
            best_word = v;
        }
    }

    delete[] vec;
    return best_word;
}


int word2vec_hs::reasoning_task_k(long long word1, long long word2, long long word3,
    int *similar_words, float *similarities, int k) {
    if (!_is_init)
        throw std::runtime_error(u8"word2vec_hs: reasoning_task: not initialized");

    float *vec = new float[_D]();
    for (int i = 0; i < _D; i++) {
        vec[i] += _W1[word2][i];
    }
    for (int i = 0; i < _D; i++) {
        vec[i] -= _W1[word1][i];
    }
    for (int i = 0; i < _D; i++) {
        vec[i] += _W1[word3][i];
    }

    int count = 0;
    for (int i = 0; i < _V; i++) {
        if (i == word1 || i == word2 || i == word3) continue;
        float sim = similarity(_W1[i], vec);
        bool inserted = false;
        if (count < k) {
            similar_words[count] = i;
            similarities[count] = sim;
            count++;
            inserted = true;
        }
        else {
            if (sim > similarities[k - 1]) {
                similar_words[k - 1] = i;
                similarities[k - 1] = sim;
                inserted = true;
            }
        }
        if (inserted) {
            for (int j = count - 1; j >= 1; j--) {
                if (similarities[j - 1] < similarities[j]) {
                    std::swap(similarities[j - 1], similarities[j]);
                    std::swap(similar_words[j - 1], similar_words[j]);
                }
            }
        }
    }
    delete[] vec;
    return std::min((long long)k, _V - 3);
}

int word2vec_hs::most_similar_k(int word, int *similar_words, float *similarities, int k) {
    int count = 0;
    for (int i = 0; i < _V; i++) {
        if (i == word) continue;
        float sim = similarity(_W1[i], _W1[word]);
        bool inserted = false;
        if (count < k) {
            similar_words[count] = i;
            similarities[count] = sim;
            count++;
            inserted = true;
        }
        else {
            if (sim > similarities[k - 1]) {
                similar_words[k - 1] = i;
                similarities[k - 1] = sim;
                inserted = true;
            }
        }
        if (inserted) {
            for (int j = count - 1; j >= 1; j--) {
                if (similarities[j - 1] < similarities[j]) {
                    std::swap(similarities[j - 1], similarities[j]);
                    std::swap(similar_words[j - 1], similar_words[j]);
                }
            }
        }
    }
    return std::min((long long)k, _V - 1);
}

float word2vec_hs::similarity(float *vec1, float *vec2) {
    float innerp = 0, norm1 = 0, norm2 = 0;
    for (int i = 0; i < _D; i++) {
        innerp += vec1[i] * vec2[i];
        norm1 += vec1[i] * vec1[i];
        norm2 += vec2[i] * vec2[i];
    }
    norm1 = sqrt(norm1);
    norm2 = sqrt(norm2);
    return innerp / (norm1 * norm2);
}

float ** word2vec_hs::alloc_2d(long long dim1, long long dim2, alloc_init_options aio) {
    float **p = new float*[dim1];
    p[0] = new float[dim1 * dim2];
    for (long long i = 1; i < dim1; i++) {
        p[i] = p[i - 1] + dim2;
    }
    switch (aio) {
    case aio_zero:
        memset(p[0], 0, sizeof(float) * dim1 * dim2);
        break;
    case aio_random:
        fill_random(p[0], dim1 * dim2);
        break;
    }
    return p;
}

void word2vec_hs::free_2d(float **p) {
    delete[] p[0];
    delete[] p;
}

void word2vec_hs::fwrite_2d(float ** p, long long dim1, long long dim2, FILE *fp) {
    for (long long i = 0; i < dim1; i++) {
        fwrite(p[i], sizeof(float), dim2, fp);
    }
}
void word2vec_hs::fread_2d(float ** p, long long dim1, long long dim2, FILE *fp) {
    for (long long i = 0; i < dim1; i++) {
        fread(p[i], sizeof(float), dim2, fp);
    }
}

void word2vec_hs::fill_random(float *p, long long count) {
    for (long long i = 0; i < count; i++) {
        int rand_val = rand();
        p[i] = (float)((float)rand_val / ((float)(RAND_MAX) / 2)) - 1;
    }
}

//void word2vec_hs::add_batch(float *dst, float *src, long long count) {
//    long long i;
//    for (long long i = 0; i < count; i++) {
//        *dst++ += *src++;
//    }
//}

void word2vec_hs::add_batch(float *dst, float *src, long long count) {
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
void word2vec_hs::fmadd_batch(float *dst, float *src1, float *src2, long long count) {
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
