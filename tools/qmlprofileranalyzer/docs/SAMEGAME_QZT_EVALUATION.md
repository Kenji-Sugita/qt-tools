# Samegame QZT Evaluation

## 対象

この文書は、`tests/data/samegame.qzt` を

- Qt Creator 19.0.0
- `qmlprofileranalyzer`

の両方で確認した結果を評価する。

参照した samegame のソースコードは次。

- [samegame.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.qml)
- [samegame.js](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.js)
- [Block.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/Block.qml)
- [Button.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/Button.qml)

## 全体評価

- `samegame.qzt` は Qt Creator 19.0.0 で正常に読み込める
- `qmlprofileranalyzer` でも `.qzt` として正常に読める
- `PixmapCache` の代表例である `background.jpg` について、
  Qt Creator の表示と `qmlprofileranalyzer` の生イベント解釈は整合している
- `events-aggregated` には Qt Creator 比較用の `qtCreatorCategory` / `qtCreatorLabel`
  などの補助フィールドが追加され、比較時に迷いにくくなった
- 一方で、Qt Creator は複数の raw event を 1 つの見やすい項目に集約して表示するため、
  完全に同じ見え方にはまだしていない
- 今回追加した `aggregate=PixmapCacheLoad` 行により、比較はかなりしやすくなった

## 主要な観察点

### 1. トレース全体

- file format は `qzt/binary`
- `traceDuration` は `20455310588 ns`
- `eventTypes=126`
- `events=10097`
- `ranges=7433`
- `pointEvents=2664`
- `notes=0`

このファイルは最小 fixture ではなく、Qt Creator 比較に十分な実トレース量を持っている。

### 2. 最長 range

- 最長 range は `samegame.qml:65`
- duration は `86627292 ns`
- 約 `86.6 ms`

対応する type は `Creating display=samegame.qml:65 details=QtQuick/Text` で、
初期 UI 生成時の `Text` 作成が最も大きい。

### 3. 次に大きい処理

`summary` と `types` から、目立つ処理は次だった。

- `samegame.qml:8`
  - `Creating`
  - `QtQuick/Rectangle`
  - 合計約 `30.3 ms`
- `samegame.qml:46`
  - `Creating`
  - `Dialog.qml`
  - 約 `30.1 ms`
- `Dialog.qml:7`
  - `Creating`
  - `QtQuick/Rectangle`
  - 約 `30.0 ms`
- `Dialog.qml:23`
  - `Creating`
  - `QtQuick/Text`
  - 約 `30.0 ms`
- `samegame.qml:19`
  - `Creating`
  - 背景 `Image`
  - 約 `18.6 ms`
- `samegame.js:33`
  - `Javascript`
  - `createBlock`
  - `count=204`
  - 合計約 `16.9 ms`
- `samegame.js:82`
  - `Javascript`
  - `floodFill`
  - `count=329`
  - 合計約 `3.55 ms`
- `samegame.js:65`
  - `Javascript`
  - `handleClick`
  - `count=9`
  - 合計約 `2.38 ms`
- `samegame.js:110`
  - `Javascript`
  - `shuffleDown`
  - `count=9`
  - 合計約 `1.20 ms`

### 4. `PixmapCache` の `background.jpg`

Qt Creator 側で選択した項目は次だった。

- `読込み済み画像`
- `ファイル: background.jpg`
- `幅: 909 px`
- `高さ: 600 px`
- `持続時間: 18.5 ms`

`qmlprofileranalyzer` 側では、これに対応する raw event が次になる。

- `4670477111  idx=79  type=background.jpg:0`
  - `cacheEventType=3`
  - 読込開始
- `4688967803  idx=80  type=background.jpg:0  height=600 width=909`
  - `cacheEventType=0`
  - サイズ判明
- `4688967803  idx=81  type=background.jpg:0`
  - `cacheEventType=4`
  - 読込完了
- `4688974577  idx=82  type=background.jpg:0  refCount=1`
  - `cacheEventType=2`
  - 参照カウント変化

今回の集約出力では、これらから次の 1 行が追加される。

- `4670477111  aggregate=PixmapCacheLoad  type=background.jpg:0  duration=18490692  file=background.jpg  width=909 height=600`

`18490692 ns` は約 `18.49 ms` で、Qt Creator の `18.5 ms` と一致する。

### 5. `SceneGraph` の観察結果

`events-aggregated` では、`SceneGraph` がフレーム種別ごとに整形される。
samegame では主に次が見えている。

- `SceneGraphRenderLoopFrame`
  - 初回の最大 `totalTiming` は `15704188 ns`
  - 約 `15.7 ms`
- `SceneGraphRendererFrame`
  - 初回の最大 `totalTiming` は `7448437 ns`
  - 約 `7.45 ms`
- `SceneGraphPolishAndSync`
  - 初回の最大 `totalTiming` は `27275584 ns`
  - 約 `27.3 ms`

このトレースでは、初回付近の `SceneGraphPolishAndSync` が最も大きく、
GUI スレッド側の polish / sync が立ち上がり時の主要コストになっている。
その後も `RenderLoopFrame`, `RendererFrame`, `PolishAndSync` が繰り返し出ており、
初期化後の描画サイクルも継続して観測できる。

### 6. `MemoryAllocation` の観察結果

`events-aggregated` では、`MemoryAllocation` が種別ごとの summary として出る。
samegame では次が確認できる。

- `HeapPage`
  - `count=2`
  - `totalAmount=712448`
  - `maxAmount=647680`
- `SmallItem`
  - `count=1399`
  - `totalAmount=693984`
  - `maxAmount=584800`
  - `minAmount=32`
- `LargeItem`
  - `count=1`
  - `totalAmount=0`

ここから、samegame のメモリイベントは
少数の大きい heap 初期化と、多数の `SmallItem` 割当てが中心だと分かる。
特に `SmallItem` の件数が多いので、盤面生成や QML オブジェクト生成に伴う
細かい割当てが支配的だと見るのが自然である。

## 解釈

### 1. この結果が示していること

- `.qzt` の読込自体は成立している
- range 再構成も成立している
- 初期 UI 生成コストと、ゲーム進行中の `Javascript` 呼び出しの両方が見えている
- `samegame.js:33 createBlock` の件数 `204` は、ブロック生成負荷がまとまって出ていることを示す
- `samegame.js:82 floodFill` と `samegame.js:110 shuffleDown` が複数回出ているので、
  クリック後のゲームロジックも追えている
- `SceneGraphPolishAndSync` の初回ピークが大きいので、
  UI 初期表示時には GUI スレッド側の同期コストも無視できない
- `MemoryAllocation` は `SmallItem` が圧倒的に多く、
  オブジェクト生成や JS 実行に伴う細かい割当てが多い

### 2. 一致している点

- `background.jpg` の読込時間
- 画像サイズ
- 読込開始から読込完了までの区間
- `SceneGraph` のフレーム種別ごとの timing 群
- `MemoryAllocation` の種別ごとの件数と量

このため、`qmlprofileranalyzer` の `.qzt` reader は
少なくとも `PixmapCache`, `SceneGraph`, `MemoryAllocation` のこのケースでは
意味解釈を大きく外していない。

### 3. そのまま性能評価に使いにくい理由

- トレースは UI 初期化とゲーム操作が混ざっている
- `samegame.qml:65` や `Dialog.qml:*` の `Creating` が大きく、
  初期表示コストの比率が高い
- `PixmapCache`, `SceneGraph`, `MemoryAllocation` は補助的に集約表示できるが、
  依然として summary や phase 別評価へは自動接続されていない
- `DebugMessage` はこのトレースでは出ていないので、
  ログ系比較まではできていない

### 4. まだ違う点

- Qt Creator は `PixmapCache` の複数 raw event を
  `読込み済み画像` のような高水準項目にまとめている
- `qmlprofileranalyzer` は raw event に加えて集約行を補助表示するが、
  Qt Creator と完全に同じ分類名や UI 表示までは再現していない
- `SceneGraph` や `MemoryAllocation` も、今は独自の summary 行であり、
  Qt Creator の統計ビューと 1 対 1 では対応しない

### 5. なぜ比較対象として有効か

- `minimal.qzt` より実トレースに近い
- `PixmapCache`, `SceneGraph`, `Javascript`, `Binding` などが十分に含まれる
- Qt Creator 上で選択した項目を、
  `qmlprofileranalyzer` の raw event と対応付けて検証できる

## コード単位の見立て

### 比較的軽い箇所

- `samegame.js:65`
  - `handleClick`
  - 1 回あたりは数百 `us` 台
- `samegame.js:110`
  - `shuffleDown`
  - 合計でも約 `1.20 ms`
- `Block.qml:16`
  - 画像 source の binding / javascript
  - 件数は多いが、1 回あたりは小さい

上の対応先:

- [samegame.js](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.js)
- [Block.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/Block.qml)

### 相対的に重い箇所

- `samegame.qml:65`
  - `QtQuick/Text` 作成
  - 最長 range
- `samegame.qml:8`
  - ルート `Rectangle` 作成
- `samegame.qml:46`
  - `Dialog.qml` 作成
- `Dialog.qml:7`, `Dialog.qml:23`
  - ダイアログ背景とテキスト生成
- `samegame.js:33`
  - `createBlock`
  - 初回盤面生成の主要コスト
- `samegame.qml:19`
  - 背景 `Image` 作成
  - `background.jpg` の pixmap load と連動して重い

上の対応先:

- [samegame.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.qml)
- [samegame.js](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.js)
- [Button.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/Button.qml)

### 現在の強い点

- `.qzt` を直接読める
- `start/end` から range を再構成できる
- `PixmapCache`, `SceneGraph`, `MemoryAllocation`, `DebugMessage` について比較用の集約行を出せる
- `Javascript` の関数単位では `startNewGame`, `createBlock`, `handleClick`, `floodFill`, `shuffleDown` まで追える

### 現在の弱い点

- 集約表示は `events-aggregated` 出力内の補助情報であり、summary には反映していない
- Qt Creator の UI 上の分類粒度と一致するわけではなく、集約は補助表示に留まる
- Qt Creator の UI 上の分類名そのものには合わせていない
- `samegame.qml` / `samegame.js` のどの操作フェーズに属するかを自動でまとめる機能はまだない

## 結論

- `samegame.qzt` は Qt Creator との照合用サンプルとして有用
- `background.jpg` の `PixmapCache` については、Qt Creator と `qmlprofileranalyzer` の結果は実質一致している
- したがって、今回の `.qzt` reader 実装は少なくともこの比較点では妥当と見てよい
- 性能面では、重いのは主に初期 UI 生成と `createBlock` による盤面生成であり、
  クリック後の `floodFill` / `shuffleDown` は相対的に軽い
- 補助情報としては、初回 `SceneGraphPolishAndSync` が約 `27.3 ms` と大きく、
  `SmallItem` allocation が `1399` 件あるため、描画同期と細粒度割当ても目立つ
- ただし、Qt Creator の表示モデルを完全再現したわけではなく、
  まだ raw event 中心のツールである点は残る

## 改善案

1. 集約表示を summary や JSON 出力にも展開して再利用しやすくする
2. `aggregate` 行を JSON 出力でも得られるようにする
3. Qt Creator の表示名に近いラベル付けを追加する
4. `startNewGame` / `createBlock` / `handleClick` / `floodFill` のフェーズ別集計を追加する
