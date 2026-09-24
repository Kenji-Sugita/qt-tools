# Executable Demo Evaluation

## 対象

この文書は、`build/localizedclockdemo` から生成した
`samples/generated/localizedclockdemo.qtd` のプロファイリング結果を評価する。

## 全体評価

- 実行形式デモとして妥当
- `qml` デモより実運用に近い
- `default.qml` 系 event が入らない分、launcher 由来のノイズは少ない
- 15秒動かすことで、`Timer` 駆動の繰り返し更新も十分観測できる
- 一方で、`Text` と locale / date / time 更新に関連する binding が目立つ

## 主要な観察点

### 1. 最長 range

- 最長 range は `Main.qml:43`
- 該当要素は `Text`
- duration は `472956334 ns`
- 約 `473 ms`

該当コード:

- `demos/localizedclock/Main.qml:43`

### 2. 次に大きい処理

- `Main.qml:56`
  - `Timer.onTriggered`
  - `count=15`
  - 合計約 `82.4 ms`
- `Main.qml:38`
  - `root.date` の `Text`
  - 約 `74.8 ms`
- `Main.qml:32`
  - 秒表示の `Text`
  - `count=16`
  - 合計約 `1.75 ms`
- `Main.qml:0`
  - `Compiling`
  - 約 `10.5 ms`
- `Main.qml:22`
  - 最初の時刻表示 `Text` 作成
  - 約 `2.15 ms`

## 解釈

### 1. この結果が示していること

- 実行形式でも QML profiler event を十分取得できている
- UI 初期生成と timer 駆動の更新が見えている
- `Binding` と `Javascript` の分離も確認できる
- 15秒実行にしたことで、起動直後だけでなく定常更新も観察しやすくなった

### 2. `qml` デモとの違い

- `qml` デモにあった `default.qml` 系 event が無い
- 実行形式アプリ本体の QML に集中した結果になっている
- そのため、実案件で `.exe` / `a.out` を見るときの感覚に近い

### 3. そのまま性能評価に使いにくい理由

- `Text` と locale / date / time 変換処理が強く効いている
- フォント警告も出ており、純粋なアプリロジックだけの負荷ではない可能性がある
- `This plugin does not support propagateSizeHints()` など、
  プラットフォームプラグイン由来の警告も混ざる

## コード単位の見立て

### 比較的軽い箇所

- `demos/localizedclock/Main.qml:50`
  - `Timer` 作成
- `demos/localizedclock/Main.qml:31`
  - 秒表示 `Text`
- `demos/localizedclock/Main.qml:37`
  - 日付表示 `Text`

### 相対的に重い箇所

- `demos/localizedclock/Main.qml:43`
  - locale 表示 `Text`
- `demos/localizedclock/Main.qml:56`
  - `Timer.onTriggered`
- `demos/localizedclock/Main.qml:32`
  - 秒表示 `Text` の binding / javascript 更新
- `demos/localizedclock/Main.qml:38`
  - 日付文字列更新

## 結論

- 実行形式デモは、`.exe` / `a.out` に近い形での reader 確認として有用
- `qml` デモより実運用寄りで、説明力も高い
- 15秒計測にしたことで時計表示サンプルらしい更新回数を確保できた
- ただし、性能ベンチマークとしてはまだ環境ノイズがある

## 改善案

1. executable demo 専用の結果差分を `qml` デモと比較する文書を追加する
2. `Main.qml:56` と `Main.qml:32` の推移を別出力で見やすくする
3. フォント依存を減らした別 executable demo を追加する
