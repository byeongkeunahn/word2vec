// main.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "util.h"
#include "wordmgr.h"
#include "word2vec.h"
#include "word2vec_hs.h"

//static const wchar_t *data_path = L"D:\\Dev\\School\\word2vec_data\\text8";
//static const wchar_t *data_path = L"D:\\Dev\\School\\word2vec_data\\text8.processed";
//static const wchar_t *data_path = L"D:\\Dev\\School\\word2vec_data\\wordset_large\\news.en-000.summary.processed.v2";
static const wchar_t *data_path = L"D:\\Dev\\School\\word2vec_data\\wordset_large\\news.en-000.summary.v3";
static const wchar_t *bin_data_path = L"D:\\Dev\\School\\word2vec_data\\wordset_large\\news.en-000.summary.v3";

//static const wchar_t *model_path = L"D:\\Dev\\School\\word2vec_data\\SG_HS_D=300.NEWS.processed";
static const wchar_t *test_path = L"D:\\Dev\\School\\word2vec_data\\questions-words-2.txt";
//static const wchar_t *model_path = L"D:\\Dev\\School\\word2vec_data\\SG_HS_D=300,K=4.param-unprocessed-MT";
//static const wchar_t *model_path = L"D:\\Dev\\School\\word2vec_data\\SG_HS_D=300,K=4.param-unprocessed-MT-SSE";
//static const wchar_t *model_path = L"D:\\Dev\\School\\word2vec_data\\SG_HS_D=300,K=5.news.processed.v3";
static const wchar_t *model_path = L"D:\\Dev\\School\\word2vec_data\\SG_HS_D=300,K=5.news.processed.v3";
static const long long D = 300;
//static const wchar_t *model_path = L"D:\\Dev\\School\\word2vec_data\\SG_HS_D=50.param";
//static const long long D = 50;

//static const wchar_t *data_path = L"D:\\Dev\\School\\word2vec_data\\debug1.txt";
//static const wchar_t *model_path = L"D:\\Dev\\School\\word2vec_data\\debug1_SG_HS.multithread";
//static const long long D = 2;

wordmgr wm;
long long V;
word2vec_hs w2v;

bool ctrl_c = false;
BOOL WINAPI my_handler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        ctrl_c = true;
        return TRUE;
    }
    return FALSE;
}

void build_corpus(wordmgr &wm) {
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

typedef struct {
    int word1;
    int word2;
    int word3;
    int word4;
} test_set;
std::vector<test_set> build_test_set(const wordmgr &wm) {
    const int buf_size = 1 << 20;

    FILE *fp = _wfopen(test_path, L"r");
    char *buf = (char *)malloc(buf_size);
    std::vector<char> v;
    std::vector<int> words;
    bool skip_this_time = false;
    std::vector<test_set> p;

    char *pos, *end;
    bool comment_mode = false;
    while (1) {
        size_t sz = fread(buf, 1, buf_size, fp);
        if (sz == 0) break;
        pos = buf;
        end = pos + sz;
        while (pos < end) {
            char c = *pos++;
            if (comment_mode) {
                comment_mode = (c != '\n');
                continue;
            }

            if (c == ':') {
                // comment line
                v.clear();
                comment_mode = true;
            }
            else if ((unsigned char)c <= 0x20) {
                if (v.size() > 0) {
                    v.push_back('\0');
                    int word_idx = wm.word_to_index(&v[0]);
                    if (word_idx == -1) {
                        skip_this_time = true;
                    }
                    words.push_back(word_idx);
                    if (words.size() == 4) {
                        if (!skip_this_time) {
                            p.push_back({ words[0], words[1], words[2], words[3] });
                        }
                        words.clear();
                        skip_this_time = false;
                    }
                    v.clear();
                }
            }
            else {
                if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
                v.push_back(c);
            }
        }
    }
    if (v.size() > 0) {
        v.push_back('\0');
        int word_idx = wm.word_to_index(&v[0]);
        if (word_idx == -1) {
            skip_this_time = true;
        }
        words.push_back(word_idx);
        if (words.size() == 4) {
            if (!skip_this_time) {
                p.push_back({ words[0], words[1], words[2], words[3] });
            }
            words.clear();
            skip_this_time = false;
        }
        v.clear();
    }
    fclose(fp);
    free(buf);
    return p;
}

void test() {
    std::vector<test_set> p = build_test_set(wm);
    printf("%lld\n", p.size());

    int correct_count = 0;
    int i;
    for (i = 1; i <= p.size() && !ctrl_c; i++) {
        test_set &ts = p[i];
        printf("%s %s %s %s\n",
            wm.index_to_word(ts.word1).c_str(),
            wm.index_to_word(ts.word2).c_str(),
            wm.index_to_word(ts.word3).c_str(),
            wm.index_to_word(ts.word4).c_str()
        );
        long long pred_word4 = w2v.reasoning_task(ts.word1, ts.word2, ts.word3);
        bool correct = ts.word4 == pred_word4;
        if (correct) {
            correct_count++;
        }
        printf("%s Expected %s, got %s\n", correct ? "Correct!" : "Wrong!  ",
            wm.index_to_word(ts.word4).c_str(), wm.index_to_word((int)pred_word4).c_str());
    }
    printf("%d correct out of %lld, ratio = %.3f\n", correct_count, p.size(), correct_count / (float)i);
}
void test_reason_k() {
    std::vector<test_set> p = build_test_set(wm);
    printf("%lld\n", p.size());

    int correct_count = 0;
    int i;
    for (i = 1; i <= p.size() && !ctrl_c; i++) {
        test_set &ts = p[i];
        printf("%s %s %s %s\n",
            wm.index_to_word(ts.word1).c_str(),
            wm.index_to_word(ts.word2).c_str(),
            wm.index_to_word(ts.word3).c_str(),
            wm.index_to_word(ts.word4).c_str()
        );

        int word_list[15];
        float similarities[15];
        w2v.reasoning_task_k(ts.word1, ts.word2, ts.word3, word_list, similarities, 15);
        for (int i = 0; i < 15; i++) {
            printf("[%s, %.10f]\n",
                wm.index_to_word(word_list[i]).c_str(), similarities[i]);
        }
        long long pred_word4 = w2v.reasoning_task(ts.word1, ts.word2, ts.word3);
        bool correct = ts.word4 == pred_word4;
        if (correct) {
            correct_count++;
        }
        printf("%s Expected %s, got %s\n", correct ? "Correct!" : "Wrong!  ",
            wm.index_to_word(ts.word4).c_str(), wm.index_to_word((int)pred_word4).c_str());
        system("pause");
    }
    printf("%d correct out of %lld, ratio = %.3f\n", correct_count, p.size(), correct_count / (float)i);
}
void train() {
    int K = 4;
    int COUNT = 100000000;

    printf("word count = %lld\n", wm.word_count());
    printf("corpus word count = %lld\n", wm.corpus_word_count());


    const long long BATCH_SIZE = 4000;
    //word_io_pair *trains = new word_io_pair[BATCH_SIZE];
    int *train_indices = new int[wm.corpus_word_count()];
    for (int i = 0; i < wm.corpus_word_count(); i++) {
        train_indices[i] = i;
    }


    long long last_save = tick64();
    for (long long i = 1; !ctrl_c; i++) {
        printf("%lldth step begin\n", i);

        std::random_device  rand_dev;
        std::mt19937        rand_generator(rand_dev());
        /*
        //long long ccTrain = wm.train_gen_sgd_batch(trains, K, BATCH_SIZE);
        //float loss = w2v.step_momentum(trains, ccTrain);
        float loss = w2v.step_skip_gram_sgd(wm, BATCH_SIZE);
        printf("%lldth minibatch, loss = %.8f\n", i, loss);
        if ((tick64() - last_save) / (double)tickfreq() >= 300) {
            w2v.save(model_path);
            last_save = tick64();
        }
        */
        //long long ccTrain = wm.train_gen_all(trains);
        //std::shuffle(trains, trains + ccTrain, rand_generator);
        long long ccTrain = wm.corpus_word_count();
        std::shuffle(train_indices, train_indices + ccTrain, rand_generator);

        auto worker_func = [&](long long start, long long end, int worker_id) {
            int print = 0;
            for (long long pos = start; pos < end && !ctrl_c; pos += BATCH_SIZE) {
                long long ccBatch = std::min(BATCH_SIZE, end - pos);
                long long tstart = tick64();
                float loss = w2v.step_skip_gram(wm, train_indices + pos, ccBatch, worker_id);
                long long tend = tick64();
                print++;
                if (print % 10 == 0) {
                    printf("[PID %d][%2.2f%%]loss = %.8f, pos=%lld, count=%lld, time=%.4f ms\n",
                        worker_id, 100.0 * (pos + ccBatch - start) / (end - start), loss, pos, ccBatch,
                        (tend - tstart) * 1000 / (double)tickfreq());
                }
            }
        };
        int num_thread = 6;
        std::vector<std::thread> threads;
        for (int i = 0; i < num_thread; i++) {
            long long start = ccTrain * i / num_thread;
            long long end = ccTrain * (i + 1) / num_thread;
            std::thread worker(worker_func, start, end, i);
            threads.push_back(std::move(worker));
        }
        for (int i = 0; i < num_thread; i++) {
            threads[i].join();
        }
        Sleep(50);
        /*
        float actual_loss = 0;
        long long ccTrainCumul = 0;
        for (long long pos = 0; pos < ccTrain && !ctrl_c; pos += BATCH_SIZE) {
            long long tick_start = tick64();

            long long ccBatch = std::min(BATCH_SIZE, ccTrain - pos);
            //float current_loss = w2v.step_momentum(trains + pos, ccBatch);
            float current_loss = w2v.step_skip_gram(wm, train_indices + pos, ccBatch);
            ccTrainCumul += ccBatch;
            actual_loss += current_loss * ccBatch;

            long long tick_end = tick64();
            printf("pos = %lld, Minibatch = %.12f, Cumul = %.12f, time=%.3f ms\n",
                pos, current_loss, actual_loss / ccTrainCumul, (tick_end - tick_start) * 1000 / (double)tickfreq());

            if ((tick_end - last_save) / (double)tickfreq() >= 300) {
                w2v.save(model_path);
                last_save = tick64();
            }
        }
        printf("Total loss = %.12f\n", actual_loss / ccTrainCumul);
        */
    }
    w2v.save(model_path);

    //delete[] trains;
    delete[] train_indices;
}

void test_sim() {
    while (!ctrl_c) {
        char buf[1000];
        scanf("%s", buf);

        int word_idx = wm.word_to_index(buf);
        if (word_idx != -1) {
            int word_list[15];
            float similarities[15];
            int count = w2v.most_similar_k(word_idx, word_list, similarities, 15);
            for (int i = 0; i < count; i++) {
                printf("[%s, %.10f]\n",
                    wm.index_to_word(word_list[i]).c_str(), similarities[i]);
            }
        }
        else {
            printf("%s not found in dictionary\n", buf);
        }
    }
}

int main() {
    SetConsoleCtrlHandler(my_handler, TRUE);
    //build_corpus(wm);
    wm.restore_from(bin_data_path);
    V = wm.word_count();
    w2v.init(V, D, &wm._src_word_freq[0]);
    w2v.set_learning_rate(0.001f);
    w2v.load(model_path);

    while (1) {
        ctrl_c = false;
        printf("1=train, 2=test, 3=test_sim, 4=test_reason_k, 5=save, 6=set_learning_rate: ");
        int choice;
        scanf("%d", &choice);
        switch (choice) {
        case 1:
            train();
            break;
        case 2:
            test();
            break;
        case 3:
            test_sim();
            break;
        case 4:
            test_reason_k();
            break;
        case 5:
            w2v.save(model_path);
            break;
        case 6:
        {
            float e;
            scanf("%f", &e);
            w2v.set_learning_rate(e);
        }
        break;
        }
    }
    //train();
    //test();
    //test_sim();
    test_reason_k();

    system("pause");
    system("pause");
    system("pause");
    system("pause");
    system("pause");
    return 0;
}
