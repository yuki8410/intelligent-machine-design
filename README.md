# Intelligent Machine Design

大学院の授業課題として、ライブラリを使わずに C 言語で実装した2つのプログラムです。

| フォルダ | 内容 |
| --- | --- |
| `BackPropergation/src` | 誤差逆伝播法による多層ニューラルネットワークの学習 |
| `RGA/src` | 実数値遺伝的アルゴリズム（RGA）による Rastrigin 関数（50次元）の最小化 |

実験結果のデータとレポートはこのリポジトリに含めていません。

## BackPropergation

層・ノード・重みを構造体で表し、順伝播・逆伝播・重み更新を実装しています。活性化関数（シグモイド、Leaky ReLU、恒等関数）は関数ポインタの組で差し替えられます。`main.c` では `sin(πx) + y` を学習対象にしています。

必要なもの：gcc、make（Linux / glibc）。グラフを描く場合は Python 3 と `requirements.txt` のパッケージ。

```sh
cd BackPropergation/src
make
./backprop
```

実行すると次のファイルを出力します。

- `rmse.csv`：1回の更新ごとの RMSE と出力
- `graph.csv`：学習後のモデルの出力（格子点）
- `correct.csv`：学習対象の関数の値（格子点）

学習回数は `main.c` の `LOOP`（既定 10,000,000）で決まります。既定値のままだと `rmse.csv` が数百MBになるので、試すときは小さくしてください。

`run.sh` はビルドから学習、グラフ描画までをまとめて実行します。`.venv` に Python の環境がある前提です。

提出後に見つかった不具合の修正内容は [`BackPropergation/修正記録.md`](BackPropergation/修正記録.md) にまとめています。

## RGA

詳しい使い方は [`RGA/src/README.MD`](RGA/src/README.MD) にあります。

必要なもの：gcc、make。グラフを描く場合は gnuplot。

```sh
cd RGA/src
make
./rga <個体数> <交叉率> <突然変異率>
```

`exp.sh` は個体数・交叉率・突然変異率の組み合わせをまとめて実行し、結果のCSVとグラフを作ります。

提出後の修正内容は [`RGA/修正記録.md`](RGA/修正記録.md) にまとめています。

## クレジット

乱数生成には Mersenne Twister（`MT.h` / `MT.c`、Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura）を使っています。ライセンスは各ファイルの冒頭に記載されています。
