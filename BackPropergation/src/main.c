#include "backprop.h"
#include <complex.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

#define INPUT_MAX 1
#define INPUT_MIN -1

#define LOOP 10000000

static inline double80 func(double80 x, double80 y)
{
    return sin(x * M_PI) + y;
}

static inline double80 weight_0()
{
    return 0;
}

static inline double80 weight_rand()
{
    // 重みの初期値を -1.0 から 1.0 の範囲でランダムに設定
    return rand_double(WEIGHT_INIT_MIN, WEIGHT_INIT_MAX);
}

void print_weights_debug(bp_t *m)
{
    if (!m) {
        fprintf(stderr, "モデルが NULL です。\n");
        return;
    }
    for (size_t i = 0; i < m->size; i++) {
        layer_t *cur  = &m->layers[i];
        layer_t *next = i < m->size - 1 ? &m->layers[i + 1] : NULL;
        // printf("Layer %zu: size %ld\n", i, cur->size);
        for (size_t j = 0; j < cur->size; j++) {
            if (i == 0) {
                printf("  Input Node %zu: value = %Lf\n", j, cur->nodes[j].result);
            } else if (i == m->size - 1) {
                printf("  Output Node %zu: result = %Lf\n", j, cur->nodes[j].result);
                continue;
            } else {
                printf("  Node %zu: result = %Lf, delta = %Lf\n", j, cur->nodes[j].result,
                       cur->nodes[j].delta);
            }
        }
        if (next) {
            weight_t **weights = m->weights[i];
            if (!weights) {
                fprintf(stderr, "Layer %zu の重みが NULL です。\n", i);
                continue;
            }
            printf("Weights to next layer:\n");
            for (size_t j = 0; j < cur->size; j++)
                for (size_t k = 0; k < next->size; k++)
                    printf("  Weight from Node %zu to Node %zu: %Lf\n", j, k, weights[j][k].value);
        }
    }
}

void handler(int sig)
{
    void *array[10];
    size_t size;
    size = backtrace(array, 10);
    if (write(STDERR_FILENO, "Error: signal received\n", 24) < 24)
        exit(1);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

int main(void)
{
    signal(SIGSEGV, handler); // セグメンテーションフォルトのハンドラを設定

    struct act_funcs funcs = {sigmoid_func, sigmoid_derivative}; // 活性化関数とその導関数
    // struct act_funcs funcs      = {relu_func, relu_derivative};
    struct act_funcs funcs_none = {none_func, none_derivative}; // 恒等関数とその導関数
    // モデルの初期化
    int node_count = 5; // 入力ノード数
    layer_t layers[7];
    layer_init(&layers[0], 3, &funcs_none);     // 入力層
    layer_init(&layers[1], node_count, &funcs); // 隠れ層
    layer_init(&layers[2], node_count, &funcs); // 隠れ層
    layer_init(&layers[3], node_count, &funcs); // 隠れ層
    layer_init(&layers[4], node_count, &funcs); // 隠れ層
    layer_init(&layers[5], node_count, &funcs); // 隠れ層
    layer_init(&layers[6], 1, &funcs_none);     // 出力層

    bp_t *model = new_bp(layers, 7, weight_rand);
    if (!model) {
        fprintf(stderr, "モデルの初期化に失敗しました。\n");
        return 1;
    }

    // 学習率の設定
    // model->learning_rate = 0.001;

    int loop_count = 0;
    int fd         = open("rmse.csv", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        goto error; // ファイルオープンエラー

    // ファイルにヘッダーを書き込む
    char *header = "time_ns,loop_count,rmse,output\n", buf[1024];
    if (write(fd, header, strlen(header)) < 0)
        goto error;

    // print_weights_debug(model); // デバッグ用に重みを表示
    do {
        double80 inputs[3] = {rand_double(INPUT_MIN, INPUT_MAX),
                              rand_double(INPUT_MIN * 0.1, INPUT_MAX * 0.1),
                              // 0}; // 入力データ
                              rand_double(INPUT_MIN, INPUT_MAX)}; // 入力データ
        double80 target[1] = {func(inputs[0], inputs[1])};
        double80 rmse      = model->update_weights(model, inputs, target); // 目標出力
        double80 *output   = model->calculate_output(model, inputs);       // モデルの出力
        sprintf(buf, "%ld,%d,%Lf,%Lf\n", (long)time(NULL), loop_count, rmse, output[0]);
        free(output); // 出力の解放
        // print_weights_debug(model); // デバッグ用に重みを表示
        // RMSEと出力をファイルに書き込む
        if (write(fd, buf, strlen(buf)) < 0)
            goto error;

        loop_count++;
    } while (loop_count < LOOP); // 学習ループ

    close(fd); // ファイルを閉じる
    // printf("学習完了: %dループ\n", loop_count);
    fd = open("graph.csv", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        goto error;

    // グラフファイルにヘッダーを書き込む
    char *graph_header = "x,y,z\n";
    if (write(fd, graph_header, strlen(graph_header)) < 0) {
        perror("グラフファイル書き込みエラー");
        close(fd);
        goto end;
    }
    // 最終的なグラフ出力
    for (int i = (INPUT_MIN) * 10; i <= (INPUT_MAX) * 10; i += 1) {
        for (int j = (INPUT_MIN) * 10; j <= (INPUT_MAX) * 10; j += 1) {
            double80 x = (double80)i / 10;
            double80 y = (double80)j / 10;
            // double80 inputs[3] = {x, y, rand_double(-1, 1)}; // 入力データ

            double80 inputs[3] = {x, y,
                                  // rand_double(INPUT_MIN, INPUT_MAX)}; // 入力データ
                                  0};
            double80 *output   = model->calculate_output(model, inputs);
            sprintf(buf, "%Lf,%Lf,%Lf\n", x, y, output[0]);
            if (write(fd, buf, strlen(buf)) < 0) {
                perror("グラフファイル書き込みエラー");
                close(fd);
                goto end;
            }

            free(output); // 出力の解放
        }
    }
    close(fd);

    fd = open("correct.csv", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        goto error;
    if (write(fd, graph_header, strlen(graph_header)) < 0) {
        perror("グラフファイル書き込みエラー");
        close(fd);
        goto end;
    }

    // 出力したいモデル
    for (int i = (INPUT_MIN) * 10; i <= (INPUT_MAX) * 10; i += 1) {
        for (int j = (INPUT_MIN) * 10; j <= (INPUT_MAX) * 10; j += 1) {
            double80 x = (double80)i / 10;
            double80 y = (double80)j / 10;
            double80 z = func(x, y);
            sprintf(buf, "%Lf,%Lf,%Lf\n", x, y, z);
            if (write(fd, buf, strlen(buf)) < 0) {
                perror("グラフファイル書き込みエラー");
                close(fd);
                goto end;
            }
        }
    }

end:
    // モデルの解放
    free_bp(model);

    return 0;
error:
    close(fd); // グラフファイルを閉じる
    perror("file error");
    exit(1); // エラー終了
}
