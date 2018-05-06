// main.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "wordmgr.h"
#include "word2vec.h"

void build_corpus(wordmgr &wm) {
    const wchar_t *data_path = L"H:\\DataSci\\word2vec_data\\text8";
    const int buf_size = 1 << 20;

    FILE *fp = _wfopen(data_path, L"r");
    char *buf = (char *)malloc(buf_size);
    std::vector<char> v;

    char *pos, *end;
    while (1) {
        size_t sz = fread(buf, 1, buf_size, fp);
        if (sz == 0) break;
        pos = buf;
        end = pos + sz;
        while (pos < end) {
            char c = *pos++;
            if ((unsigned char)c <= 0x20) {
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

    wm.freeze();
}

int main() {
    wordmgr wm;
    build_corpus(wm);

    long long V = wm.word_count();
    long long D = 300;
    int K = 5;
    int COUNT = 100;

    word2vec w2v;
    w2v.init(V, D);
    w2v.set_learning_rate(0.1);

    long long ccTrainBound = 2 * V*K;
    long long *in = new long long[ccTrainBound];
    long long *out = new long long[ccTrainBound];

    for (int i = 0; i < COUNT; i++) {
        printf("%dth step begin\n", i);
        wm.train_gen_init(K);
        long long ccTrain;
        while (ccTrain = wm.train_gen(5, in, out)) {
            double current_loss = w2v.step(in, out, ccTrain);
            printf("%.12f\n", current_loss);
        }
    }
    system("pause");

    delete[] in;
    delete[] out;

    return 0;
}
