#ifndef BACKPROP_H
#define BACKPROP_H

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#define LEARNING_RATE_DEFAULT 0.001 // デフォルトの学習率

#define WEIGHT_INIT_MAX 1.0  // 重みの初期値の最大値
#define WEIGHT_INIT_MIN -1.0 // 重みの初期値の最小値

typedef long double double80;

static inline double80 relu_func(double80 x)
{
    return x > 0 ? x : 0.01 * x; // ReLU関数（Leaky ReLU）
}
static inline double80 relu_derivative(double80 x)
{
    return x > 0 ? 1 : 0.01; // ReLUの導関数（Leaky ReLU）
}

static inline double80 sigmoid_func(double80 x)
{
    return 1.0 / (1.0 + exp(-x)); // シグモイド関数
}
static inline double80 sigmoid_derivative(double80 x)
{
    return x * (1.0 - x); // シグモイドの導関数
}

static inline double80 none_func(double80 x)
{
    return x; // 恒等関数
}
static inline double80 none_derivative(double80 x)
{
    return 1.0; // 恒等関数の導関数
}

double80 rand_double(double min, double max);

struct act_funcs {
    double80 (*activation)(double80 x); // 活性化関数
    double80 (*derivative)(double80 x); // 導関数
};

// 重み構造体
typedef struct weight {
    double80 value; // 重みの値
} weight_t;

// ノード（ニューロン）
typedef struct node {
    double80 result;         // 出力値
    double80 delta;          // 誤差項
    struct act_funcs *funcs; // 活性化関数とその導関数

} node_t;

// レイヤー（層）
typedef struct layer {
    node_t *nodes; // ノード配列
    size_t size;   // ノード数
} layer_t;

// バックプロパゲーションモデル本体
typedef struct bp {
    layer_t *layers;     // レイヤー配列
    weight_t ***weights; // 重み配列（各レイヤー間の重み）
    size_t size;         // レイヤー数
    void (*__forward)(struct bp *, const double80 *inputs);  // 順伝播関数
    void (*__backward)(struct bp *, const double80 *target); // 逆伝播関数
    double80 (*update_weights)(struct bp *, const double80 *inputs, const double80 *target);
    double80 *(*calculate_output)(struct bp *, const double80 *inputs);
    double80 (*root_mean_square_error)(struct bp *, const double80 *target); // RMSE計算関数
    double80 learning_rate;                                                  // 学習率

} bp_t;

void layer_init(layer_t *l, size_t size, struct act_funcs *funcs);
void layer_free(layer_t *l);

// モデル生成 / 解放
bp_t *new_bp(layer_t *layers, size_t size, double80(weight_init)());
void free_bp(bp_t *m);

#endif // BACKPROP_H
