
#pragma once

#pragma pack(1)
typedef struct {
    double v[1];
} my_vector;
#pragma pack()

class word2vec {
public:
    word2vec();
    ~word2vec();

public:
    void init(long long V, long long D);
    void load(const wchar_t *path);
    void save(const wchar_t *path);
    void destroy();

public:
    void set_learning_rate(double e);
    // SGD, mini-batch or full-batch depending on ccTrain supplied
    // indices should be 0-based
    // returns current value of the loss function
    double step(const long long *in, const long long *out, long long ccTrain);

private:
    void add_batch(double *dst, double *src, long long count, double mul);

private:
    bool _is_init;
    long long _V;
    long long _D;
    double _e;
    double *_W1_transp; // V x D
    double *_W2_transp; // D x V
    double *_grad_W1_L;
    double *_grad_W2_L;

    // for convenient access
    inline double& _W1(long long v, long long d) {
        return _W1_transp[d * _V + v];
    }
    inline double& _W2(long long d, long long v) {
        return _W2_transp[v * _D + d];
    }
    inline double& _W1_grad(long long v, long long d) {
        return _grad_W1_L[d * _V + v];
    }
    inline double& _W2_grad(long long d, long long v) {
        return _grad_W2_L[v * _D + d];
    }
};

