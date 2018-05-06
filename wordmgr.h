
#pragma once

class wordmgr
{
public:
    wordmgr();
    ~wordmgr();

public:
    void append(const char *word);
    void freeze();
    bool is_frozen();

public:
    long long word_count();
    void train_gen_init(int neighbors);
    long long train_gen(long long desired_count, long long *in, long long *out);

private:
    bool _frozen;
    std::map<std::string, int> _dict;
    std::vector<int> _word_freq;
    std::vector<int> _words;
    int _next_idx;

    struct {
        int neighbors;
        int next_center;
        int next_offset;
    } _state;
};

