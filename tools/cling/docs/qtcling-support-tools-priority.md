# Qt Cling Support Tools Priority

Qt Cling の支援ツール候補を、Cling / ROOT の一次情報を踏まえて見直した優先順位。

## 優先度

1. `Widget Sandbox / Test Bench`
   - 単体 widget を最小構成で立ち上げ、その場で試すための場。
   - Qt Cling の主用途に最も合う。

2. `Live Snippet Runner`
   - 実行中 widget / object に短い C++/Qt コードを増分投入して試す。
   - Qt Cling 固有の強みを最も直接使う機能。

3. `Session History to Script/Source`
   - REPL で試した内容を再実行可能な `.cpp` や script に整理して残す。
   - ROOT の macro workflow に最も近い支援。

4. `Object-Specific Editors`
   - `Property Editor` の次として、`Layout Editor`、`Style Editor`、`Model/View Editor` などを追加する方向。
   - ROOT の object-specific editor に相当。

5. `Tool Browser / Plugin Host`
   - 各支援ツールを dock / tab にまとめる母体。
   - ツール群が揃ってから価値が大きくなる。

6. `Visual Builder + Save to Code`
   - 視覚的に組んでコード化する支援。
   - 価値はあるが実装は重い。

7. `Qt Test Export`
   - 必要時のみ使う後段の補助機能。
   - Qt Cling の中心価値ではない。

## 判断軸

- Qt Cling の強みは、GUI を表示したまま C++/Qt をその場で試せること。
- ROOT の実践では、`試す -> 残す -> 再実行する` ループが強い。
- そのため、まず `試せる場` を優先し、その後に `増分コード実行`、さらに `履歴の保存` を置く。
