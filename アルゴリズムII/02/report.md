# 課題2
## 赤黒木

### プログラム
- [main.c](main.c) (動作確認用)
- [rbtree.c](rbtree.c)
- [rbtree.h](rbtree.h)

### 関数ごとのフローチャート

#### `rb_create`
```mermaid
flowchart TD
A[rb_create start] --> B[tree を確保]
B --> C{確保成功?}
C -- no --> D[perror -> exit]
C -- yes --> E[nil ノードを確保]
E --> F{確保成功?}
F -- no --> D
F -- yes --> G[nil を黒に設定]
G --> H[root = nil]
H --> I[tree を返す]
```

#### `rb_search`
```mermaid
flowchart TD
A[rb_search start] --> B[node = root]
B --> C{node != nil かつ key != node.key?}
C -- no --> D[node を返す]
C -- yes --> E{key < node.key?}
E -- yes --> F[node = left]
E -- no --> G[node = right]
F --> C
G --> C
```

#### `rb_insert`
```mermaid
flowchart TD
A[rb_insert start] --> B[新規ノード z を作成]
B --> C[y = nil, x = root]
C --> D{x != nil?}
D -- no --> E[z の親を y に設定]
D -- yes --> F{z.key < x.key?}
F -- yes --> G[x = left]
F -- no --> H[x = right]
G --> I[y = x]
H --> I
I --> D
E --> J{y == nil?}
J -- yes --> K[root = z]
J -- no --> L{z.key < y.key?}
L -- yes --> M[y.left = z]
L -- no --> N[y.right = z]
K --> O[rb_insert_fixup]
M --> O
N --> O
O --> P[終了]
```

#### `rb_insert_fixup`
```mermaid
flowchart TD
A[rb_insert_fixup start] --> B{親が赤?}
B -- no --> Z[root を黒]
B -- yes --> C{親が左子?}
C -- yes --> D[叔父 y = 右子]
C -- no --> E[叔父 y = 左子]

D --> F{叔父が赤?}
E --> F

F -- yes --> G[親と叔父を黒]
G --> H[祖父を赤]
H --> I[z = 祖父]
I --> B

F -- no --> J{内側ケース?}
J -- yes --> K[回転して外側ケースへ]
K --> L[親を黒]
L --> M[祖父を赤]
M --> N[祖父へ回転]
N --> Z
J -- no --> L
Z --> O[終了]
```

#### `rb_left_rotate`
```mermaid
flowchart TD
A[rb_left_rotate start] --> B[y = x.right]
B --> C[x.right = y.left]
C --> D{y.left != nil?}
D -- yes --> E[y.left.parent = x]
D -- no --> F[次へ]
E --> F
F --> G[y.parent を x.parent に接続]
G --> H{x は根?}
H -- yes --> I[root = y]
H -- no --> J{x は左子?}
J -- yes --> K[parent.left = y]
J -- no --> L[parent.right = y]
I --> M[y.left = x]
K --> M
L --> M
M --> N[x.parent = y]
N --> O[終了]
```

#### `rb_right_rotate`
```mermaid
flowchart TD
A[rb_right_rotate start] --> B[x = y.left]
B --> C[y.left = x.right]
C --> D{x.right != nil?}
D -- yes --> E[x.right.parent = y]
D -- no --> F[次へ]
E --> F
F --> G[x.parent を y.parent に接続]
G --> H{y は根?}
H -- yes --> I[root = x]
H -- no --> J{y は左子?}
J -- yes --> K[parent.left = x]
J -- no --> L[parent.right = x]
I --> M[x.right = y]
K --> M
L --> M
M --> N[y.parent = x]
N --> O[終了]
```

#### `rb_transplant`
```mermaid
flowchart TD
A[rb_transplant start] --> B{u は根?}
B -- yes --> C[root = v]
B -- no --> D{u は左子?}
D -- yes --> E[u.parent.left = v]
D -- no --> F[u.parent.right = v]
C --> G[v.parent = u.parent]
E --> G
F --> G
G --> H[終了]
```

#### `rb_minimum`
```mermaid
flowchart TD
A["rb_minimum start"] --> B{"node.left != nil?"}
B -- yes --> C["node = left"]
B -- no --> D["node を返す"]
C --> B
```

#### `rb_delete`
```mermaid
flowchart TD
A["rb_delete start"] --> B["削除対象 z を search"]
B --> C{"z == nil?"}
C -- yes --> D["return"]
C -- no --> E["y = z, y色を保存"]
E --> F{"left が nil?"}
F -- yes --> G["x = right, transplant"]
F -- no --> H{"right が nil?"}
H -- yes --> I["x = left, transplant"]
H -- no --> J["後継 y = minimum(right)"]
J --> K["y色を保存, x = y.right"]
K --> L{"y.parent == z?"}
L -- yes --> M["x.parent = y"]
L -- no --> N["transplant(y, y.right)"]
N --> O["y.right = z.right"]
O --> P["y.right.parent = y"]
M --> Q["transplant(z, y)"]
P --> Q
Q --> R["y.left = z.left"]
R --> S["y.left.parent = y"]
S --> T["y.color = z.color"]
I --> U["free(z)"]
G --> U
T --> U
U --> V{"元の色は黒?"}
V -- yes --> W["delete_fixup"]
V -- no --> X["終了"]
W --> X
```

#### `rb_delete_fixup`
```mermaid
flowchart TD
A["rb_delete_fixup start"] --> B{"x != root かつ x が黒?"}
B -- no --> Z["x を黒"]
B -- yes --> C{"x は左子?"}
C -- yes --> D["w = 兄弟(右)"]
C -- no --> E["w = 兄弟(左)"]

D --> F{"w が赤?"}
E --> F
F -- yes --> G["w を黒, 親を赤"]
G --> H["親へ回転"]
H --> I["w を更新"]
F -- no --> I

I --> J{"w の両子が黒?"}
J -- yes --> K["w を赤"]
K --> L["x = 親"]
L --> B
J -- no --> M{"外側の子が黒?"}
M -- yes --> N["内側の子を黒, w を赤"]
N --> O["w へ回転"]
O --> P["w を更新"]
M -- no --> P
P --> Q["w の色を親に合わせる"]
Q --> R["親を黒にする"]
R --> S["外側の子を黒にする"]
S --> T["親へ回転"]
T --> Z
Z --> U["終了"]
```

#### `rb_free`
```mermaid
flowchart TD
A["rb_free start"] --> B{"tree == NULL?"}
B -- yes --> C["return"]
B -- no --> D["rb_free_nodes(root)"]
D --> E["nil を解放"]
E --> F["tree を解放"]
F --> G["終了"]
```

#### `rb_print`
```mermaid
flowchart TD
A["rb_print start"] --> B{"root == nil?"}
B -- yes --> C["(empty) を表示"]
B -- no --> D["右部分木を再帰表示"]
D --> E["自ノードを表示"]
E --> F["左部分木を再帰表示"]
F --> G["終了"]
```

### 実行結果
`test.sh` でコンパイルと実行を行った。  
結果は `result.txt` に保存した。

#### 挿入の経過

`7` 挿入後
```mermaid
flowchart TB
N7["7(B)"]
```

`4` 挿入後
```mermaid
flowchart TB
N7["7(B)"] -->|L| N4["4(R)"]
```

`3` 挿入後
```mermaid
flowchart TB
N4["4(B)"]
N4 -->|L| N3["3(R)"]
N4 -->|R| N7["7(R)"]
```

`1` 挿入後
```mermaid
flowchart TB
N4["4(B)"]
N4 -->|L| N3["3(B)"]
N4 -->|R| N7["7(B)"]
N3 -->|L| N1["1(R)"]
```

`5` 挿入後
```mermaid
flowchart TB
N4["4(B)"]
N4 -->|L| N3["3(B)"]
N4 -->|R| N7["7(B)"]
N3 -->|L| N1["1(R)"]
N7 -->|L| N5["5(R)"]
```

`2` 挿入後
```mermaid
flowchart TB
N4["4(B)"]
N4 -->|L| N2["2(B)"]
N4 -->|R| N7["7(B)"]
N2 -->|L| N1["1(R)"]
N2 -->|R| N3["3(R)"]
N7 -->|L| N5["5(R)"]
```

#### 探索の結果
```text
search 3: found (3,R)
search 6: not found
```

#### 削除の経過

`2` 削除後
```mermaid
flowchart TB
N4["4(B)"]
N4 -->|L| N3["3(B)"]
N4 -->|R| N7["7(B)"]
N3 -->|L| N1["1(R)"]
N7 -->|L| N5["5(R)"]
```

`4` 削除後
```mermaid
flowchart TB
N5["5(B)"]
N5 -->|L| N3["3(B)"]
N5 -->|R| N7["7(B)"]
N3 -->|L| N1["1(R)"]
```

`7` 削除後
```mermaid
flowchart TB
N5["5(B)"]
N5 -->|L| N3["3(B)"]
N3 -->|L| N1["1(B)"]
```

`3` 削除後
```mermaid
flowchart TB
N5["5(B)"]
N5 -->|L| N1["1(R)"]
```

`5` 削除後
```mermaid
flowchart TB
N1["1(B)"]
```

`1` 削除後
```mermaid
flowchart TB
E["(empty)"]
```

### 解説
- 赤黒木は二分探索木に色の制約を追加した構造で、挿入と削除のたびに回転と再配色で平衡を保つ。
- 挿入では、まず赤で追加してから親・叔父・祖父の関係を見て修正する。
- 削除では、削除対象の色が黒のときだけ黒高さの崩れを修正する。
- 探索は通常の二分探索木と同じで、値の大小だけで左右に降りていく。
