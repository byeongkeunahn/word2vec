
#pragma once

typedef struct {
    int in;
    int out;
} word_io_pair;

typedef struct {
    int in;
    int out[8];
    int out_window_sz;
} te_skip_gram;
typedef struct {
    int in[8];
    int out;
    int in_window_sz;
} te_cbow;

class wordmgr
{
public:
    wordmgr();
    ~wordmgr();

public:
    void restore_from(const wchar_t *path);
    void append(const char *word);
    void drop_infrequent_words();
    void freeze();
    bool is_frozen() const;

public:
    long long word_count(); // size of dictionary
    long long corpus_word_count(); // size of corpus
    void train_gen_init(int neighbors);
    long long train_gen(long long desired_count, word_io_pair *buf);
    long long train_gen_all(word_io_pair *buf);
    long long train_gen_sgd_batch(word_io_pair *buf, int K, long long desired_count);
    //long long train_gen_skip_gram(te_skip_gram *buf, int K, long long desired_count);
    int word_to_index(const char *word) const;
    std::string index_to_word(int idx) const;

public:
    bool _frozen;
    std::map<std::string, int> _src_dict;
    std::vector<std::string> _src_word_dict;
    std::vector<int> _src_word_freq;
    std::vector<int> _src_words;
    std::vector<int> _src_negative_unigram_3_4;

    int _next_idx;

    struct _state_type {
        int neighbors;
        int next_center;
        int next_offset;
    } _state;
};

