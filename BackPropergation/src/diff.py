import csv
import numpy as np

def read_third_column(filename):
    with open(filename, newline='') as csvfile:
        reader = csv.reader(csvfile)
        next(reader)  # ヘッダーをスキップ
        return np.array([float(row[2]) for row in reader])

def root_mean_squared_error(pred, correct):
    mse = np.mean((pred - correct) ** 2)
    rmse = np.sqrt(mse)
    return rmse

# ファイルからデータを読み込む
graph_z = read_third_column('graph.csv')
correct_z = read_third_column('correct.csv')

# RMSEを計算
rmse = root_mean_squared_error(graph_z, correct_z)

print(f"二乗誤差のルート（RMSE）: {rmse}")
