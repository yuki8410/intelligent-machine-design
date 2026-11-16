#!/bin/bash

# ==============================================================================
# 実数値GA実験自動化スクリプト (個別グラフ生成機能付き)
# ==============================================================================

# --- 事前チェック ---
if ! command -v gnuplot &> /dev/null
then
    echo "[ERROR] gnuplot が見つかりません。グラフを作成するにはインストールが必要です。"
    echo "  (例: sudo apt-get install gnuplot / brew install gnuplot)"
    exit 1
fi

# --- パラメータ設定 (ここを編集して実験条件を変更) ---
INDIVIDUALS_LIST="100 200 1000"       # 個体数
CROSSOVER_RATES="0.3 0.5 0.8"          # 交叉率
MUTATION_RATES="0.01 0.05 0.1"         # 突然変異率

# --- 初期設定 ---
RESULTS_DIR="results"           # CSVデータ保存用
GRAPHS_DIR="graphs"             # 個別グラフ保存用
LOG_FILE="summary_log.txt"      # 実行時間サマリー
COMPARISON_PLOT_FILE="fitness_comparison.png" # 比較用グラフ

# 結果保存用ディレクトリを作成
mkdir -p "$RESULTS_DIR"
mkdir -p "$GRAPHS_DIR"

# 既存のログファイルをクリア
> "$LOG_FILE"
echo "--- RGA Experiment Log ---" > "$LOG_FILE"
echo "Date: $(date)" >> "$LOG_FILE"
echo "--------------------------------" >> "$LOG_FILE"


# --- メインループ ---
echo "[INFO] 実験を開始します..."

for individuals in $INDIVIDUALS_LIST; do
  for crossover_rate in $CROSSOVER_RATES; do
    for mutation_rate in $MUTATION_RATES; do
      
      param_string="ind${individuals}_cross${crossover_rate}_mut${mutation_rate}"
      printf "\n[RUN] パラメータ: %s\n" "$param_string"
      
      # プログラムを実行し、実行時間を抽出
      output=$(./rga "$individuals" "$crossover_rate" "$mutation_rate" 2>&1)
      exec_time=$(echo "$output" | grep "completed in" | awk '{print $(NF-1)}')
      
      if [ -z "$exec_time" ]; then
        echo "[WARN] 実行時間の抽出に失敗しました。"
        exec_time="N/A"
      fi

      echo "$output"
      echo "[INFO] 実行時間: ${exec_time}s"

      # fitness.csv が存在するか確認
      if [ -f "fitness.csv" ]; then
        # CSVファイルをリネームして移動
        result_csv_file="${RESULTS_DIR}/fitness_${param_string}.csv"
        mv fitness.csv "$result_csv_file"
        echo "[INFO] 結果を ${result_csv_file} に保存しました。"
        
        # ログファイルに記録
        echo "params: ${param_string}, execution_time: ${exec_time}s" >> "$LOG_FILE"
        sync
        individual_plot_file="${GRAPHS_DIR}/graph_${param_string}.png"
        # gnuplotに渡すコマンドを変数に格納
        GNUPLOT_CMD="
            set terminal pngcairo size 800,600 enhanced font 'Verdana,10';
            set output '$individual_plot_file';
            set title 'Fitness vs. Generation (Ind: $individuals, Cross: $crossover_rate, Mut: $mutation_rate)';
            set xlabel 'Generation';
            set ylabel 'Fitness';
            set grid;
            set datafile separator ',';
            set logscale y;
            set key off;

            set format y '%.0t×10^{%L}';
       
            plot '$result_csv_file' using 1:2 with lines title 'Fitness';
        "

        # gnuplotを実行
        gnuplot -e "$GNUPLOT_CMD"

        if [ -f "$individual_plot_file" ]; then
            echo "[INFO] 個別グラフを ${individual_plot_file} に保存しました。"
        else
            echo "[ERROR] 個別グラフの作成に失敗しました: ${individual_plot_file}"
        fi

      else
        echo "[ERROR] fitness.csv が生成されませんでした。スキップします。"
        echo "params: ${param_string}, status: FAILED" >> "$LOG_FILE"
      fi

    done
  done
done

echo ""
echo "[INFO] 全ての個別実験が完了しました。"

echo ""
echo "全ての処理が完了しました。"
echo "  - 実行ログ: $LOG_FILE"
echo "  - 生データ: $RESULTS_DIR/ ディレクトリ"
echo "  - 個別グラフ: $GRAPHS_DIR/ ディレクトリ"
