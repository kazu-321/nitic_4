# 課題4
## 整数線形計画法によるつけ麺の売上最大化

### フローチャート

#### `pivot_table`
```mermaid
flowchart TD
A[pivot_table start] --> B[ピボットの逆数を計算]
B --> C[ピボット行と列以外を更新]
C --> D[ピボット行を正規化]
D --> E[ピボット列を更新]
E --> F[基底変数と非基底変数を交換]
F --> G[end]
```

#### `simplex_phase`
```mermaid
flowchart TD
A[simplex_phase start] --> B[目的行を決める]
B --> C[入る列を選ぶ]
C --> D{改善できる列があるか}
D -- no --> E[成功で終了]
D -- yes --> F[出る行を比率で選ぶ]
F --> G{出る行があるか}
G -- no --> H[非有界として失敗]
G -- yes --> I[pivot_table を実行]
I --> C
```

#### `solve_simplex`
```mermaid
flowchart TD
A[solve_simplex start] --> B[基底変数と表を初期化]
B --> C[制約と目的関数を表に入れる]
C --> D[右辺が最小の行を探す]
D --> E{右辺が負か}
E -- no --> J[phase2 へ進む]
E -- yes --> F[人工変数を入れて phase1]
F --> G{phase1 成功か}
G -- no --> H[実行不能として終了]
G -- yes --> I[人工変数を外す]
I --> J
J --> K{phase2 成功か}
K -- no --> L[非有界として終了]
K -- yes --> M[解を基底変数から復元]
M --> N[目的値を返す]
```

#### `calc_z`
```mermaid
flowchart TD
A[calc_z start] --> B[目的関数の各係数を掛ける]
B --> C[総和を取る]
C --> D[値を返す]
```

#### `check_original_constraints`
```mermaid
flowchart TD
A[check_original_constraints start] --> B[各制約式を順に確認]
B --> C[lhs を計算]
C --> D{右辺を超えるか}
D -- yes --> E[false を返す]
D -- no --> F{次の制約があるか}
F -- yes --> B
F -- no --> G[true を返す]
```

#### `branch_and_bound`
```mermaid
flowchart TD
A[branch_and_bound start] --> B[node_count を増やす]
B --> C[LP 緩和を simplex で解く]
C --> D{実行可能か}
D -- no --> E[戻る]
D -- yes --> F{既知最良より良い上界か}
F -- no --> E
F -- yes --> G{整数解か}
G -- yes --> H[整数解として記録]
H --> E
G -- no --> I[分岐変数を選ぶ]
I --> J[下側制約を追加]
J --> K[再帰呼び出し]
K --> L[上側制約を追加]
L --> M[再帰呼び出し]
M --> E
```

#### `main`
```mermaid
flowchart TD
A[main start] --> B[3 変数の目的関数を設定]
B --> C[制約行列と上限を設定]
C --> D[LP 緩和を解く]
D --> E[LP 緩和の結果を表示]
E --> F[分枝限定法で整数解を探索]
F --> G[整数解と探索節点数を表示]
G --> H[end]
```

### プログラム
- [ramen.cpp](ramen.cpp) 動作確認用の実装
- [ramen_all.cpp](ramen_all.cpp) 全探索による検算用

### 実行結果
- [test.sh](test.sh) で `ramen.cpp` をコンパイルして実行した。
- 実行結果は [result.txt](result.txt) に保存した。
- [test_ramen_all.sh](test_ramen_all.sh) で `ramen_all.cpp` をコンパイルして実行した。
- 実行結果は [ramen_all_result.txt](ramen_all_result.txt) に保存した。

`ramen.cpp` の実行結果では、LP 緩和の解が

- `x1 = 35.5555555556`
- `x2 = 6.6666666667`
- `x3 = 20.0000000000`
- `z = 58555.5555555556`

となり、整数制約を満たさないことが分かった。

その後、分枝限定法で整数解を探索した結果、

- `x1 = 35`
- `x2 = 7`
- `x3 = 20`
- `z = 58400`

を得た。

`ramen_all.cpp` の全探索結果も

- `豚鶏Wつけ麺: 35個`
- `鶏特盛つけ麺: 7個`
- `デラックスつけ麺: 20個`
- `最大売上: 58400円`

となり、分枝限定法の結果と一致した。

### 解説
- `pivot_table` はピボット操作の本体で、表の更新と基底変数の入れ替えを行う。
- `simplex_phase` は単体法の1段階分を進める処理で、改善できる列と出る行を選びながらピボットを繰り返す。
- `solve_simplex` は表の初期化から二段階シンプレックス法の実行、最終的な解の復元までをまとめて行う。
- `calc_z` は整数解 `x` から目的関数値を計算する。
- `check_original_constraints` は、丸めた整数解が元の制約を満たしているかを確認する。
- `branch_and_bound` は、LP 緩和の解が整数でないときに分岐して再帰的に探索し、最良の整数解を更新する。
- `main` は問題設定、LP 緩和の表示、分枝限定法による整数解探索、結果表示を順に実行する。
- `ramen_all.cpp` は全探索で同じ問題を解き、`58400` 円が最適値であることを検算している。
