#include "backprop.h"
#include "MT.h" // Mersenne Twister ランダム数生成器
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SEED 0 // 乱数初期化用のシード値
#define unlikely(x) (__builtin_expect(!!(x), 0))

void (*init_rand)(long unsigned int) = init_genrand; // 乱数初期化関数
double80 rand_double(double min, double max)
{
    if (unlikely(init_rand)) {
        init_rand(SEED);  // 初回呼び出し時のみ SEED で乱数を初期化
        init_rand = NULL; // 初期化後は NULL に設定
    }
    return (double80)genrand_real1() * (max - min) + min;
}

// レイヤー初期化: ノード配列を確保し、各ノードの初期値を設定
void layer_init(layer_t *l, size_t size, struct act_funcs *funcs)
{
    if (!l) {
        fprintf(stderr, "layer_init: layer is NULL\n");
        exit(1); // レイヤーがNULLの場合はエラー
    }
    if (size == 0) {
        fprintf(stderr, "layer_init: size is 0, nothing to do\n");
        exit(1); // サイズが0の場合はエラー
    }
    // ノード確保
    l->nodes = calloc(size, sizeof(node_t));
    l->size  = size;
    for (size_t i = 0; i < size; i++) {
        l->nodes[i].result = 0.0;
        l->nodes[i].delta  = 0.0;
        l->nodes[i].funcs  = funcs; // 活性化関数の設定
    }
}

// レイヤー解放
void layer_free(layer_t *l)
{
    if (!l)
        return;
    free(l->nodes);

    l->nodes = NULL;
    l->size  = 0;
}

// モデル解放
void free_bp(bp_t *m)
{
    if (!m)
        return;

    for (size_t i = 0; i < m->size - 1; i++) {
        layer_t *cur       = &m->layers[i];
        weight_t **weights = m->weights[i];
        for (size_t j = 0; j < cur->size; j++)
            free(weights[j]); // 各ノードの重み配列を解放
        free(weights);        // 重み配列自体を解放
    }
    free(m->weights); // 全ての重み配列を解放
    for (size_t i = 0; i < m->size; i++)
        layer_free(&m->layers[i]);
    free(m);
}

double80 bp_rmse(bp_t *m, const double80 *target)
{
    if (!m || !target || m->size < 2)
        return 0.0;
    layer_t *output_layer      = &m->layers[m->size - 1];
    double80 sum_squared_error = 0.0;
    for (size_t i = 0; i < output_layer->size; i++) {
        double80 error = output_layer->nodes[i].result - target[i];
        sum_squared_error += error * error;
    }
    return sqrt(sum_squared_error / output_layer->size);
}
// フォワードパス
void bp_forward(bp_t *m, const double80 *input)
{
    if (!m || !input || m->size < 2)
        return;
    // 入力層の初期化
    layer_t *input_layer = &m->layers[0];
    for (size_t i = 0; i < input_layer->size; i++)
        input_layer->nodes[i].result = input[i];

    // printf("bp_forward: input layer initialized\n");
    // printf("bp_forward: starting forward pass\n");
    // 順伝播計算
    for (size_t li = 1; li < m->size; li++) {
        // printf("bp_forward: processing layer %zu\n", li);
        layer_t *prev      = &m->layers[li - 1];
        layer_t *cur       = &m->layers[li];
        weight_t **weights = m->weights[li - 1];
        for (size_t j = 0; j < cur->size; j++) {
            double80 sum = 0.0;
            // 重み付き和
            for (size_t i = 0; i < prev->size; i++)
                sum += prev->nodes[i].result * weights[i][j].value;
            cur->nodes[j].result = cur->nodes[j].funcs->activation(sum); // 活性化関数
        }
    }
}

void bp_backward(bp_t *m, const double80 *target)
{

    size_t L        = m->size;
    layer_t *output = &m->layers[L - 1];
    // printf("bp_backward: starting backward pass\n");
    // 出力層の誤差項
    for (size_t i = 0; i < output->size; i++) {
        double80 y             = output->nodes[i].result;
        double80 dy            = (y - target[i]) * output->nodes[i].funcs->derivative(y);
        output->nodes[i].delta = dy;
    }
    // auto derivative
    for (size_t li = L - 1; li-- > 1;) {
        layer_t *cur       = &m->layers[li];
        layer_t *next      = &m->layers[li + 1];
        weight_t **weights = m->weights[li];

        for (size_t i = 0; i < cur->size; i++) {
            double80 sum_delta = 0.0;
            for (size_t j = 0; j < next->size; j++)
                sum_delta += weights[i][j].value * next->nodes[j].delta;
            cur->nodes[i].delta = sum_delta * cur->nodes[i].funcs->derivative(cur->nodes[i].result);
        }
    }
    // printf("bp_backward: backward pass completed\n");
    // printf("bp_backward: updating weights\n");
    // 重み更新
    for (size_t li = 0; li < L - 1; li++) {
        layer_t *cur       = &m->layers[li];
        layer_t *nxt       = &m->layers[li + 1];
        weight_t **weights = m->weights[li];

        for (size_t i = 0; i < cur->size; i++) {
            for (size_t j = 0; j < nxt->size; j++) {
                weight_t *w = &weights[i][j];
                w->value -= m->learning_rate * nxt->nodes[j].delta * cur->nodes[i].result;
            }
        }
    }
}

double80 update_weights(bp_t *m, const double80 *input, const double80 *target)
{
    if (!m || !input || !target)
        return 0;
    // 順伝播
    // printf("update_weights: starting forward pass\n");
    m->__forward(m, input);
    // printf("update_weights: forward pass completed\n");

    // 誤差
    // printf("update_weights: calculating RMSE\n");
    double80 rmse = m->root_mean_square_error(m, target);
    // printf("update_weights: RMSE calculated: %Lf\n", rmse);

    // 逆伝播
    // printf("update_weights: starting backward pass\n");
    m->__backward(m, target);
    // printf("update_weights: backward pass completed\n");

    return rmse; // RMSEを返す
}

double80 *calculate_output(bp_t *m, const double80 *input)
{
    if (!m || !input)
        return NULL;
    // 順伝播実行
    m->__forward(m, input);
    // 出力層の結果を返す
    layer_t *output_layer = &m->layers[m->size - 1];
    double80 *output      = malloc(output_layer->size * sizeof(*output));
    if (!output)
        return NULL;
    for (size_t i = 0; i < output_layer->size; i++)
        output[i] = output_layer->nodes[i].result;
    return output;
}

// モデル生成
bp_t *new_bp(layer_t *layers, size_t size, double80(weight_init)())
{
    if (!layers || size < 2)
        return NULL;
    bp_t *m                   = calloc(1, sizeof(bp_t));
    m->layers                 = layers;
    m->size                   = size;
    m->__forward              = bp_forward;            // 順伝播関数
    m->__backward             = bp_backward;           // 逆伝播関数
    m->update_weights         = update_weights;        // 重み更新関数
    m->calculate_output       = calculate_output;      // 出力計算関数
    m->root_mean_square_error = bp_rmse;               // RMSE計算関数
    m->learning_rate          = LEARNING_RATE_DEFAULT; // デフォルト学習率

    // weightsの初期化
    m->weights = calloc(size - 1, sizeof(weight_t **));
    if (!m->weights)
        goto free; // メモリ確保失敗
    // 各レイヤー間の重みを確保
    for (size_t i = 0; i < size - 1; i++) {
        // printf("new_bp: initializing weights for layer %zu - %zu\n", i, i + 1);
        layer_t *cur       = &m->layers[i];
        layer_t *next      = &m->layers[i + 1];
        weight_t **weights = calloc(cur->size, sizeof(weight_t *));
        if (!weights)
            goto free; // メモリ確保失敗
        for (size_t j = 0; j < cur->size; j++) {
            weights[j] = calloc(next->size, sizeof(weight_t));
            if (!weights[j])
                goto free; // メモリ確保失敗
            // 重みの初期化
            for (size_t k = 0; k < next->size; k++) {
                weights[j][k].value =
                    weight_init ? weight_init() : 0.0; // 初期化関数が指定されていれば使用
            }
        }
        m->weights[i] = weights; // 重み配列を設定
    }

    return m;
free:
    fprintf(stderr, "new_bp: memory allocation failed\n");
    free_bp(m);
    return NULL; // メモリ確保失敗
}
