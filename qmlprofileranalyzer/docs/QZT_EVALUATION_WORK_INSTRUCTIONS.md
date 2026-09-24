# QZT Evaluation Work Instructions

## 目的

この文書は、`.qzt` と対応するソースコードが与えられたときに、
`qmlprofileranalyzer` を使って評価文書を作成するための運用向け指示書である。

対象成果物の例:

- `..._QZT_EVALUATION.md`
- 必要に応じて `..._QZT_PROCEDURE.md`

## 必要な入力

最低限必要なもの:

1. `.qzt` ファイル
2. 対応するソースコード

あると望ましいもの:

- Qt Creator での表示確認結果
- 対応する `.qml`, `.js`, 必要なら `.cpp`
- どの観点を重視するか
  - 互換性確認
  - 重い処理の把握
  - Qt Creator との差分確認

## 基本方針

- まず `.qzt` 全体を読む
- 次に `types` と `events` でコード位置とイベント意味を結び付ける
- 必要なら Qt Creator 表示と照合する
- 最後に、重い処理、軽い処理、解釈、制約を文書化する

## 実施手順

### 1. 入力の整合確認

確認項目:

- `.qzt` が読めるか
- ソースコードの系統が `.qzt` と対応しているか
- ファイル名と行番号が大きくずれていないか

### 2. 全体統計の取得

```bash
./build/qmlprofileranalyzer summary <trace.qzt>
```

記録する項目:

- `file format`
- `traceDuration`
- `eventTypes`
- `events`
- `ranges`
- `pointEvents`
- `notes`
- `longestRange`

### 3. type 情報の取得

```bash
./build/qmlprofileranalyzer types <trace.qzt>
```

見る項目:

- `Creating`
- `Binding`
- `Javascript`
- `HandlingSignal`
- `PixmapCache`
- `details`
- `file=...:line:column`

### 4. raw event と集約行の確認

```bash
./build/qmlprofileranalyzer events <trace.qzt>
./build/qmlprofileranalyzer events-aggregated <trace.qzt>
```

見る項目:

- raw event が必要なら `events`
- `# aggregates`
- 集約比較が必要なら `events-aggregated`
- 画像読込などの集約行
- `SceneGraph` のフレーム種別ごとの集約
- `MemoryAllocation` の種別ごとの summary
- `DebugMessage` の level 別 summary
- 対応する raw event

### 5. ソースコード対応付け

確認項目:

- `summary` / `types` の行番号がどのコードに対応するか
- `details` が実コード内容と整合しているか
- 重い処理が初期化か、更新か、ロジックか

### 6. Qt Creator 比較

必要な場合:

- `Load QML Trace` で `.qzt` を開く
- 任意のイベントを選択して
  - 名前
  - duration
  - 補助属性
  を `qmlprofileranalyzer` 出力と照合する

### 7. 評価文作成

最低限含める項目:

1. 全体評価
2. 主要な観察点
3. 最長 range
4. 次に大きい処理
5. Qt Creator と一致している点
6. まだ違う点
7. 比較的軽い箇所
8. 相対的に重い箇所
9. 必要なら `SceneGraph` / `MemoryAllocation` / `DebugMessage` の観察結果
10. 結論
11. 改善案

## 出力文書の推奨構成

- `対象`
- `全体評価`
- `主要な観察点`
- `解釈`
- `コード単位の見立て`
- `結論`
- `改善案`

## 注意点

- `.qzt` とソースコードのバージョンがずれると、行番号対応が崩れる
- Qt Creator と `qmlprofileranalyzer` は表示モデルが同一ではない
- 評価文は自動生成ではなく、人が観察結果を解釈してまとめる

## 関連文書

- [QZT Evaluation Procedure](/Users/sugita/src/qt/tools/qmlprofileranalyzer/docs/SAMEGAME_QZT_PROCEDURE.md)
- [Samegame QZT Evaluation](/Users/sugita/src/qt/tools/qmlprofileranalyzer/docs/SAMEGAME_QZT_EVALUATION.md)
