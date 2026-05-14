# 課題1
## MSP
### フローチャート
```mermaid
flowchart TD
A[main start] --> B[call max_subarray]
B --> C[print result]
C --> D[main end]

subgraph S[max_subarray from num]
  E[enter function] --> F{num == 1}
  F -- yes --> G[return data from]
  F -- no --> H{num == 2}

  H -- yes --> I[set max with one or two elements]
  I --> J[return max]

  H -- no --> K[split into left and right]
  K --> L[recurse left]
  L --> M[recurse right]
  M --> N[init cross values]
  N --> O[scan left suffix max]
  O --> P[scan right prefix max]
  P --> Q[calc cross max]
  Q --> R[max = left max]
  R --> T{right max > max}
  T -- yes --> U[update max with right]
  T -- no --> V{cross max > max}
  U --> V
  V -- yes --> W[update max with cross]
  V -- no --> X[return max]
  W --> X
end

B -. call .-> E
L -. recursive call .-> E
M -. recursive call .-> E
G -. return .-> C
J -. return .-> C
X -. return .-> C
```

### プログラム
- [msp.c](msp/msp.c)
- [msp.h](msp/msp.h)

### テストデータ
- [data_0](msp/data/data_0.c)
- [data_1](msp/data/data_1.c)
- [data_2](msp/data/data_2.c)
- [data_3](msp/data/data_3.c)

### 実行結果
[test.sh](msp/test.sh)でコンパイルと実行を行いました。

[期待する実行結果](msp/data/excepted.txt)

[実際の実行結果](msp/data/result.txt)

diffコマンドで比較した結果、期待する実行結果と実際の実行結果は同一でした。

### 解説
- numが1のときは、その値が最大なので、data[from]を返します。
- numが2のときは、data[from]とdata[from + 1]とdata[from] + data[from + 1]の最大値を返します。
- numが2より大きいときは、配列を半分に分割して、左側と右側の最大値を再帰的に求めます。
- また、左側の末尾からの最大値と右側の先頭からの最大値を求めて、それらの和をcross_maxとします。
- 各cross_maxは一時的に端の値を入れているが、範囲に含まれる要素ならなんでもよいです。
- sumの値midから離れるように追加されています。
- 最後に、左側の最大値、右側の最大値、cross_maxの中で最大の値を返します。
- data_1~3は自分で作成したテストデータで、kadaneのアルゴリズムで求めた最大値を期待する実行結果に記載しています。


## Dijkstra
### フローチャート
```mermaid
flowchart TD
A[main start] --> B[dist 0 = 0]
B --> C[dist 1..n-1 = INF]
C --> D[new_heap n]
D --> E[heap val i = i]
E --> F{heap num != 0}

F -- yes --> G[xm = remove_root]
G --> H[i = 0]
H --> I{i < heap num}
I -- no --> F
I -- yes --> J[xi = heap val i]
J --> K{weight xm xi == 0}
K -- yes --> L[i = i + 1]
K -- no --> M[sum = dist xm + weight]
M --> N{sum <= dist xi}
N -- yes --> O[dist xi = sum]
O --> P[change_heap heap i]
P --> L
N -- no --> L
L --> I

F -- no --> Q[xi = 0]
Q --> R{xi < num n}
R -- yes --> S[print xi and dist]
S --> T[xi = xi + 1]
T --> R
R -- no --> U[end]
```

```mermaid
flowchart TD
A[new_heap num] --> B[malloc HEAP]
B --> C{malloc ok}
C -- no --> D[perror and exit]
C -- yes --> E[set heap num]
E --> F[calloc val array]
F --> G[return heap]

H[remove_root heap] --> I[root = val 0]
I --> J[val 0 = val num-1 and num--]
J --> K[heapify heap 0]
K --> L[return root]

M[change_heap heap pos] --> N{pos == 0 or pos >= num}
N -- yes --> O[return]
N -- no --> P[parent = pos-1 / 2]
P --> Q{dist pos < dist parent}
Q -- yes --> R[swap pos and parent]
R --> S[change_heap parent]
Q -- no --> O

T[heapify heap pos] --> U[left right smallest]
U --> V{left in range and smaller}
V -- yes --> W[smallest = left]
V -- no --> X
W --> X{right in range and smaller}
X -- yes --> Y[smallest = right]
X -- no --> Z
Y --> Z{smallest != pos}
Z -- yes --> AA[swap pos and smallest]
AA --> AB[heapify smallest]
Z -- no --> AC[return]
AB --> AC
```

### プログラム
- [dijkstra.c](dijkstra/dijkstra.c)
- [dijkstra.h](dijkstra/dijkstra.h)
- [heap.c](dijkstra/heap.c)
- [heap.h](dijkstra/heap.h)

### テストデータ
- [data_0](dijkstra/data/data_0.c)
- [data_1](dijkstra/data/data_1.c)
- [data_2](dijkstra/data/data_2.c)
- [data_3](dijkstra/data/data_3.c)

### 実行結果
[test.sh](dijkstra/test.sh)でコンパイルと実行を行いました。

[期待する実行結果](dijkstra/data/excepted.txt)

[実際の実行結果](dijkstra/data/result.txt)

diffコマンドで比較した結果、期待する実行結果と実際の実行結果は同一でした。

### 解説
- dist配列は、各ノードへの最短距離を保持します。
- 最初に、dist[0]を0に設定し、他のノードへの距離を無限大に設定します。
- ヒープを作成し、各ノードをヒープに追加します。
- ヒープが空になるまで、最小の距離を持つノードをヒープから取り出し、そのノードから隣接するノードへの距離を更新します。
- 更新された距離が現在の距離より小さい場合、dist配列を更新し、ヒープ内の位置を変更します。
- 最後に、各ノードへの最短距離を出力します。
- data_1~3は自分で作成したテストデータで、PythonでDijkstraのアルゴリズムを実装して求めた最短距離を期待する実行結果に記載しています。
