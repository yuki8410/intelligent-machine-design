import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def split_stats(lst, k):
    """
    リスト lst を k 分割し、
    各チャンクの平均値と標準偏差を返す。
    """
    n = len(lst)
    base = n // k
    rem = n % k
    means, stds = [], []
    start = 0
    for i in range(k):
        size = base + (1 if i < rem else 0)
        chunk = lst[start:start+size]
        means.append(np.mean(chunk))
        stds.append(np.std(chunk, ddof=0))  # 母集団標準偏差(ddof=0)
        start += size
    return means, stds

# CSV 読み込み
df = pd.read_csv('rmse.csv')

# 分割数
k = 10

# 各カラム（X, Y）について平均と標準偏差を取得
x_vals = df.iloc[:,1].values
y_vals = df.iloc[:,2].values
x = [(i+1)*len(x_vals)/k for i in range(k)]
y_means, y_stds = split_stats(y_vals, k)

# プロット
plt.figure(figsize=(8, 6))

# 平均値に標準偏差のエラーバーを描画
plt.errorbar(
    x, y_means,
    yerr=y_stds,      # 標準偏差をエラーバーとして使用
    fmt='o-',          # マーカーと直線
    capsize=5,         # エラーバーの先端の横線長
    lw=2,              # 線の太さ
    label='mean ± STD'
)

plt.xlabel(df.columns[1])
plt.ylabel(df.columns[2])
plt.title(f'{df.columns[2]} vs {df.columns[1]} (+ std)')
plt.grid(True)
plt.savefig('rmse_plot_with_std.png', dpi=300)

