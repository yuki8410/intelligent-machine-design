import csv
import matplotlib.pyplot as plt
import sys
import numpy as np
from mpl_toolkits.mplot3d import Axes3D

x_list = []
y_list = []
z_list = []

file = sys.argv[1]

# CSV読み込み
with open(file, 'r', newline='') as csvfile:
    reader = csv.reader(csvfile)
    header = next(reader, None)
    for row in reader:
        if len(row) < 3:
            print(f"Skipping invalid row: {row}")
            continue
        try:
            xi, yi, zi = map(float, row[:3])
            x_list.append(xi)
            y_list.append(yi)
            z_list.append(zi)
        except ValueError:
            print(f"Skipping non-numeric row: {row}")

print(f"読み込んだデータ点の数: {len(x_list)}")

# numpy 配列に変換
x_arr = np.array(x_list)
y_arr = np.array(y_list)
z_arr = np.array(z_list)

# グリッドを作成する（重複なしのx, y値が必要）
x_unique = np.unique(x_arr)
y_unique = np.unique(y_arr)
X, Y = np.meshgrid(x_unique, y_unique)

# Zを再構成（注意：x, yに対応するzの並びが必要）
Z = np.empty_like(X)
for i in range(len(y_unique)):
    for j in range(len(x_unique)):
        # 各 (x, y) に対する z を探す
        mask = (x_arr == x_unique[j]) & (y_arr == y_unique[i])
        if np.any(mask):
            Z[i, j] = z_arr[mask][0]
        else:
            Z[i, j] = np.nan  # データがなければ NaN

# 3D サーフェス描画
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
surf = ax.plot_surface(X, Y, Z, cmap='viridis', edgecolor='none')

# ラベル
ax.set_xlabel('X axis')
ax.set_ylabel('Y axis')
ax.set_zlabel('Z axis')

# カラーバー追加（任意）
fig.colorbar(surf)

# 画像保存
outfile = file.replace(".csv", ".png")
plt.savefig(outfile, dpi=300, bbox_inches='tight')
plt.close(fig)
