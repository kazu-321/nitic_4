# 課題3

## 2-3木

### フローチャート

#### `min_keys_for_height(height)`
```mermaid
graph TD
    A[開始] --> B[keys を 1 にする]
    B --> C{高さ分 繰り返す}
    C -->|はい| D[keys を 2 倍して 1 を足す]
    D --> C
    C -->|いいえ| E[keys を返す]
```

#### `max_keys_for_height(height)`
```mermaid
graph TD
    A[開始] --> B[keys を 2 にする]
    B --> C{高さ分 繰り返す}
    C -->|はい| D[keys を 3 倍して 2 を足す]
    D --> C
    C -->|いいえ| E[keys を返す]
```

#### `destroy(node)`
```mermaid
graph TD
    A[開始] --> B{node は null か}
    B -->|はい| C[終了]
    B -->|いいえ| D[子ノードを順に破棄]
    D --> E[node を delete]
    E --> C
```

#### `partition_count(total, parts, min_part, max_part, out)`
```mermaid
graph TD
    A[開始] --> B{範囲内か判定}
    B -->|いいえ| C[false を返す]
    B -->|はい| D[out を最小値で初期化]
    D --> E[余りを計算]
    E --> F[i を 0 にする]
    F --> G{i は parts 未満か}
    G -->|いいえ| H{余りは 0 か}
    H -->|はい| I[true を返す]
    H -->|いいえ| C
    G -->|はい| J[残りと余りから割当量を決める]
    J --> K[out の i 番目に加える]
    K --> L[余りを減らす]
    L --> M[i を増やす]
    M --> G
```

#### `build_subtree(arr, l, r, height)`
```mermaid
graph TD
    A[開始] --> B[n を計算]
    B --> C{n は 0 か}
    C -->|はい| D[null を返す]
    C -->|いいえ| E[新しい node を作る]
    E --> F{height は 0 か}
    F -->|はい| G[区間の値を葉に入れて返す]
    F -->|いいえ| H[子の最小と最大を計算]
    H --> I[1キー2分岐を試す]
    I --> J[2キー3分岐を試す]
    J --> K{有効な候補があるか}
    K -->|いいえ| L[ノードを破棄して例外]
    K -->|はい| M{1キー構成か}
    M -->|はい| N[左子と右子を再帰生成]
    N --> O[区切り値を 1 個設定]
    O --> P[子の親を設定]
    P --> Q[node を返す]
    M -->|いいえ| R[左 中 右 子を再帰生成]
    R --> S[区切り値を 2 個設定]
    S --> T[子の親を設定]
    T --> Q
```

#### `search_node(node, key)`
```mermaid
graph TD
    A[開始] --> B{node は null か}
    B -->|はい| C[false を返す]
    B -->|いいえ| D[ノード内の全キーを確認]
    D --> E{key と一致するか}
    E -->|はい| F[true を返す]
    E -->|いいえ| G{葉ノードか}
    G -->|はい| C
    G -->|いいえ| H{キーが 1 個か}
    H -->|はい| I[左か右へ再帰]
    H -->|いいえ| J[3 分岐を選んで再帰]
```

#### `rebuild()`
```mermaid
graph TD
    A[開始] --> B[root を破棄して null にする]
    B --> C{values は空か}
    C -->|はい| D[終了]
    C -->|いいえ| E[必要な高さを求める]
    E --> F[build_subtree で root を再構築]
    F --> G{root はあるか}
    G -->|はい| H[root の parent を null にする]
    G -->|いいえ| D
    H --> D
```

#### `repeat_char(c, n)`
```mermaid
graph TD
    A[開始] --> B[c を n 個並べた文字列を作る]
    B --> C[返す]
```

#### `to_utf8(s)`
```mermaid
graph TD
    A[開始] --> B[UTF-32 文字列を UTF-8 に変換]
    B --> C[返す]
```

#### `make_label(node)`
```mermaid
graph TD
    A[開始] --> B[左括弧を追加]
    B --> C[キーを順に追加]
    C --> D{まだキーがあるか}
    D -->|はい| C
    D -->|いいえ| E[右括弧を追加]
    E --> F[UTF-32 文字列にして返す]
```

#### `render_box(node)`
```mermaid
graph TD
    A[開始] --> B{node は null か}
    B -->|はい| C[空の箱を返す]
    B -->|いいえ| D[label を作る]
    D --> E{葉ノードか}
    E -->|はい| F[label だけを 1 行で返す]
    E -->|いいえ| G[子ノードを再帰描画]
    G --> H[子の数に応じて配置を決める]
    H --> I[親と子の中心を計算]
    I --> J[必要幅と位置を決める]
    J --> K[上段にラベルと罫線を描く]
    K --> L[接続線を描く]
    L --> M[子の描画結果を横に合成]
    M --> N[箱を返す]
```

#### `print_rendered(box)`
```mermaid
graph TD
    A[開始] --> B[各行を順に処理]
    B --> C[末尾の空白を削除]
    C --> D[UTF-8 に変換して出力]
    D --> B
    B --> E[終了]
```

#### `TwoThreeTree::~TwoThreeTree()`
```mermaid
graph TD
    A[終了処理開始] --> B[root を破棄]
    B --> C[終了]
```

#### `insert(key)`
```mermaid
graph TD
    A[開始] --> B[挿入位置を探す]
    B --> C{同じ値があるか}
    C -->|はい| D[終了]
    C -->|いいえ| E[values に挿入]
    E --> F[rebuild で作り直す]
    F --> G[終了]
```

#### `search(key)`
```mermaid
graph TD
    A[開始] --> B[search_node を呼ぶ]
    B --> C[結果を返す]
```

#### `print_search_trace(key)`
```mermaid
graph TD
    A[開始] --> B[探索開始メッセージを出す]
    B --> C{root は空か}
    C -->|はい| D[空木の not found を出して終了]
    C -->|いいえ| E[print_search_trace_node を呼ぶ]
    E --> F[終了]
```

#### `print()`
```mermaid
graph TD
    A[開始] --> B{root は空か}
    B -->|はい| C[空表示して終了]
    B -->|いいえ| D[render_box を作る]
    D --> E[print_rendered で出力]
    E --> F[終了]
```

#### `print_search_trace_node(node, key, depth)`
```mermaid
graph TD
    A[開始] --> B[現在ノードを表示]
    B --> C[ノード内の全キーを確認]
    C --> D{key と一致するか}
    D -->|はい| E[found を出して終了]
    D -->|いいえ| F{葉ノードか}
    F -->|はい| G[not found を出して終了]
    F -->|いいえ| H[次に進む子を決める]
    H --> I{キーが 1 個か}
    I -->|はい| J[左か右を選ぶ]
    I -->|いいえ| K[左 中 右 から選ぶ]
    J --> L[go メッセージを出す]
    K --> L
    L --> M[選んだ子へ再帰]
```

#### `remove(key)`
```mermaid
graph TD
    A[開始] --> B[削除対象を探す]
    B --> C{見つかったか}
    C -->|いいえ| D[終了]
    C -->|はい| E[values から削除]
    E --> F[rebuild で作り直す]
    F --> G[終了]
```

#### `main()`
```mermaid
graph TD
    A[開始] --> B[奇数列を用意]
    B --> C[挿入順をシャッフル]
    C --> D[TwoThreeTree を作る]
    D --> E{挿入を最後まで処理したか}
    E -->|いいえ| F[1 個挿入して木を表示]
    F --> E
    E -->|はい| G[検索トレースと検索結果を表示]
    G --> H[削除順を別にシャッフル]
    H --> I{削除を最後まで処理したか}
    I -->|いいえ| J[1 個削除して木を表示]
    J --> I
    I -->|はい| K[終了]
```

### 解説



## パトリシア木

### フローチャート

#### `makeEndMarker()`
```mermaid
graph TD
    A[開始] --> B[# を 1 個だけ持つ描画用ノードを返す]
```

#### `trimRight(row)`
```mermaid
graph TD
    A[開始] --> B{末尾が空白か}
    B -->|はい| C[末尾を 1 文字削除]
    C --> B
    B -->|いいえ| D[終了]
```

#### `putText(grid, row, col, text)`
```mermaid
graph TD
    A[開始] --> B[text の各文字を順に処理]
    B --> C{書き込み位置が範囲内か}
    C -->|はい| D[grid に 1 文字ずつ格納]
    C -->|いいえ| E[その文字は無視]
    D --> B
    E --> B
    B --> F[終了]
```

#### `cellWidth(text)`
```mermaid
graph TD
    A[開始] --> B{罫線文字か}
    B -->|はい| C[幅 1 を返す]
    B -->|いいえ| D[文字列長を返す]
```

#### `putToken(grid, row, col, token)`
```mermaid
graph TD
    A[開始] --> B{位置が範囲内か}
    B -->|はい| C[grid に token を置く]
    B -->|いいえ| D[何もしない]
    C --> E[終了]
    D --> E
```

#### `renderTree(node, is_root)`
```mermaid
graph TD
    A[開始] --> B{node はあるか}
    B -->|いいえ| C[空の描画結果を返す]
    B -->|はい| D[label を決める]
    D --> E{葉ノードか}
    E -->|はい| F[label だけを 1 行で返す]
    E -->|いいえ| G[子ノードを再帰描画]
    G --> H[子の数に応じて配置を決める]
    H --> I[親ラベルと子の中心を計算]
    I --> J[必要な幅と位置を決める]
    J --> K[上段にラベルと横線を描画]
    K --> L[接続行に縦線を描画]
    L --> M[子の描画結果を横に合成]
    M --> N[描画結果を返す]
```

#### `erase(node, word, depth)`
```mermaid
graph TD
    A[開始] --> B[各子を順に見る]
    B --> C[子ラベルと word の一致長を調べる]
    C --> D{ラベル全体が一致したか}
    D -->|いいえ| B
    D -->|はい| E{word の末尾まで来たか}
    E -->|はい| F{子が終端か}
    F -->|いいえ| G[削除失敗で false]
    F -->|はい| H[終端フラグを下ろす]
    H --> I{子に子ノードがないか}
    I -->|はい| J[子を削除して親から除去]
    J --> K[true を返す]
    I -->|いいえ| L{子が 1 個だけか}
    L -->|はい| M[孫を持ち上げてラベルを結合]
    M --> K
    L -->|いいえ| K
    E -->|いいえ| N[一致した子へ再帰]
    N --> O{再帰が成功したか}
    O -->|いいえ| G
    O -->|はい| P{子が空になったか}
    P -->|はい| J
    P -->|いいえ| Q{子が 1 個だけか}
    Q -->|はい| M
    Q -->|いいえ| K
```

#### `PatriciaTree::PatriciaTree()`
```mermaid
graph TD
    A[開始] --> B[root に空ノードを確保]
    B --> C[終了]
```

#### `PatriciaTree::~PatriciaTree()`
```mermaid
graph TD
    A[終了処理開始] --> B[root を deleteTree で破棄]
    B --> C[終了]
```

#### `deleteTree(node)`
```mermaid
graph TD
    A[開始] --> B{node はあるか}
    B -->|いいえ| C[終了]
    B -->|はい| D[全子を順に deleteTree]
    D --> E[node を delete]
    E --> C
```

#### `insert(word)`
```mermaid
graph TD
    A[開始] --> B[node を root にする]
    B --> C{i は word の長さ未満か}
    C -->|いいえ| D[終端を true にして終了]
    C -->|はい| E[先頭文字の子を探す]
    E -->|いいえ| F[残り文字列を新しい子として追加]
    F --> G[終了]
    E -->|はい| H[既存子ラベルと word を比較]
    H --> I{ラベル全体が一致したか}
    I -->|はい| J[その子へ進み i を進める]
    J --> C
    I -->|いいえ| K[途中で分岐する split を作る]
    K --> L[既存子のラベルを後半に切る]
    L --> M[split の子に既存子を付ける]
    M --> N{残りの word があるか}
    N -->|はい| O[新しい葉を追加]
    N -->|いいえ| P[split を終端にする]
    O --> G
    P --> G
```

#### `search(word)`
```mermaid
graph TD
    A[開始] --> B[node を root にする]
    B --> C{i は word の長さ未満か}
    C -->|いいえ| D[終端を返す]
    C -->|はい| E[先頭文字の子を探す]
    E -->|いいえ| F[false を返す]
    E -->|はい| G[子ラベルと word を比較]
    G --> H{ラベル全体が一致したか}
    H -->|いいえ| F
    H -->|はい| I[その子へ進み i を進める]
    I --> C
```

#### `startsWith(prefix)`
```mermaid
graph TD
    A[開始] --> B[node を root にする]
    B --> C{i は prefix の長さ未満か}
    C -->|いいえ| D[true を返す]
    C -->|はい| E[先頭文字の子を探す]
    E -->|いいえ| F[false を返す]
    E -->|はい| G[子ラベルと prefix を比較]
    G --> H{prefix として矛盾するか}
    H -->|はい| F
    H -->|いいえ| I[その子へ進み i を進める]
    I --> C
```

#### `erase(word)`
```mermaid
graph TD
    A[開始] --> B[root から erase を呼ぶ]
    B --> C[結果を返す]
```

#### `print()`
```mermaid
graph TD
    A[開始] --> B[root を renderTree で描画]
    B --> C[各行を文字列にまとめて表示]
    C --> D[終了]
```

#### `main()`
```mermaid
graph TD
    A[開始] --> B[挿入用の単語列を準備]
    B --> C[挿入順を乱数でシャッフル]
    C --> D[PatriciaTree を作る]
    D --> E{挿入を最後まで処理したか}
    E -->|いいえ| F[1 個挿入して木を表示]
    F --> E
    E -->|はい| G[検索結果を表示]
    G --> H[削除用の単語列を準備]
    H --> I{削除を最後まで処理したか}
    I -->|いいえ| J[1 個削除して木を表示]
    J --> I
    I -->|はい| K[終了]
```

### 解説

