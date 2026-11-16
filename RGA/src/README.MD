# About
実数値遺伝的アルゴリズム (RGA) の実装です。


# requirements
- C23 コンパイラ
- `make`
- gnuplot


# Usage

make
./rga <individuals> <crossover_rate> <mutation_rate>

世代数を指定する場合は、main.c の  `LOOP` を変更してください。

# Output
- `fitness.csv`: 各世代の最適解の適応度を記録した CSV ファイル
