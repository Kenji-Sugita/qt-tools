# Demo Evaluation

## 対象

この文書は、実行デモ `samples/qml/AutoTrace.qml` から生成した
`samples/generated/autotrace.qtd` のプロファイリング結果を評価する。

## 全体評価

- このデモは軽量な QML サンプルとして妥当
- 生成された実トレースでは `events=423`, `ranges=24`
- ただし event の多くは `MemoryAllocation`, `SceneGraph`, `AnimationFrame` であり、
  QML ロジック本体は初期化時に集中している
- `qml` ベースの実行では `default.qml` 由来の初期化 event も追加で含まれる

## 主要な観察点

### 1. 最長 range

- 最長 range は `AutoTrace.qml:35`
- 該当要素は `Text`
- duration は `508066625 ns`
- 約 `508 ms`

該当コード:

- `samples/qml/AutoTrace.qml:35`

### 2. 次に大きい処理

- `AutoTrace.qml:0`
  - `Compiling` 相当
  - 約 `8.2 ms`
- `AutoTrace.qml:4`
  - `Window` 作成
  - 約 `0.50 ms`
- `AutoTrace.qml:17`
  - `Rectangle` 作成
  - 約 `0.034 ms`
- `AutoTrace.qml:26`
  - `NumberAnimation` 作成
  - 約 `0.049 ms`
- `AutoTrace.qml:43`
  - `Timer` 作成
  - 約 `0.019 ms`

補足:

- `default.qml:0`, `default.qml:5`, `default.qml:6` も追加で観測される
- これは `qml` ランタイム設定の初期化由来と見られる

## 解釈

### 1. この結果が示していること

- QML の主要イベントは正しく取得できている
- `Compiling`, `Creating`, `Binding`, `Javascript`, `MemoryAllocation`,
  `SceneGraph`, `AnimationFrame`, `DebugMessage` が見えている
- 解析ツールの `.qtd` reader は実トレースにも対応できている
- `qml` 実行時に追加される `default.qml` 系 event も読めている

### 2. この結果をそのまま性能評価に使いにくい理由

- `Text` の約 `445 ms` は、このデモ規模に対して大きすぎる
- そのため、純粋に QML コードが重いというより、環境要因が混ざっている可能性が高い

混入していそうな要因:

- 初回フォント解決
- オフスクリーン実行
- 初回 text rendering 初期化
- scene graph 初期化
- profiler 計測時の初回コスト

実行時にも次の警告が出ている:

- font family alias の警告

特にフォント警告は `Text` の初期化時間に影響している可能性がある

## コード単位の見立て

### 問題が小さい箇所

- `samples/qml/AutoTrace.qml:26`
  - `NumberAnimation`
  - 軽い
- `samples/qml/AutoTrace.qml:43`
  - `Timer`
  - 軽い
- `samples/qml/AutoTrace.qml:17`
  - `Rectangle`
  - 軽い

### 相対的に重い箇所

- `samples/qml/AutoTrace.qml:35`
  - `Text`
  - このデモ結果の主観測点

## 結論

- このデモは、QML profiler event を一通り取得できることの確認には十分
- 一方で、性能ベンチマーク用デモとしてはまだ粗い
- 現状の結果は「解析ツールの動作確認」には向くが、
  「QML コードの性能評価」には環境ノイズが大きい
- `qml` ベースへ切り替えたことで、現行推奨に沿ったデモになった
- その代わり、`qmlscene` 時とは異なり `default.qml` 由来の event が混ざる

## 改善案

1. `Text` を外したデモを追加する
2. 使用フォントを明示したデモを追加する
3. animation / binding / input event / pixmap cache を個別に確認するデモを増やす
4. Qt Creator 側の表示結果と件数・duration を照合する
