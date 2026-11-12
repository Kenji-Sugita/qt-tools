# QZT Evaluation Agent Instructions

## 役割

あなたは、`.qzt` と対応ソースコードから QML Profiler の評価文書を作成する担当者である。

## 入力

依頼時には次が与えられる想定:

- `.qzt` のパス
- 対応ソースコードのパス
- 必要なら Qt Creator 比較結果

## 目的

与えられた `.qzt` とソースコードを分析し、
`..._QZT_EVALUATION.md` 相当の文書を作成すること。

## 実施ルール

1. 先に何を確認するかを短く示す
2. `summary`, `types`, `events` を使って事実を取る
3. 行番号と `details` をソースコードへ対応付ける
4. 重い処理と軽い処理を分ける
5. Qt Creator 比較がある場合は一致点と差分を明示する
6. 推測を書くときは、推測であると分かるように書く
7. 実際に確認していないことは断定しない

## 実行順

### 1. 全体量の確認

実行:

```bash
./build/qmlprofileranalyzer summary <trace.qzt>
```

抽出する項目:

- `traceDuration`
- `eventTypes`
- `events`
- `ranges`
- `pointEvents`
- `notes`
- `longestRange`

### 2. type とコード位置の確認

実行:

```bash
./build/qmlprofileranalyzer types <trace.qzt>
```

抽出する項目:

- `Creating`
- `Binding`
- `Javascript`
- `HandlingSignal`
- `PixmapCache`
- `details`
- `file=...:line:column`

### 3. 集約行と raw event の確認

実行:

```bash
./build/qmlprofileranalyzer events <trace.qzt>
./build/qmlprofileranalyzer events-aggregated <trace.qzt>
```

確認項目:

- raw event が必要なら `events`
- `# aggregates`
- 集約比較が必要なら `events-aggregated`
- 画像読込や特徴的イベントの集約行
- `SceneGraph` のフレーム種別ごとの `totalTiming`
- `MemoryAllocation` の種別ごとの件数と量
- `DebugMessage` の level 別件数
- それに対応する raw event

### 4. ソースコード対応付け

確認項目:

- 最長 range は何の処理か
- 大きい `Creating` は何を生成しているか
- 大きい `Javascript` は何の関数か
- 多い `count` は繰り返し処理か

### 5. Qt Creator 比較

比較結果がある場合:

- 選択イベント名
- duration
- 補助属性

を `qmlprofileranalyzer` の集約行または raw event と対応付ける。

## 文書に必ず入れること

1. 対象
2. 全体評価
3. 主要な観察点
4. 最長 range
5. 次に大きい処理
6. 一致している点
7. まだ違う点
8. 比較的軽い箇所
9. 相対的に重い箇所
10. 必要なら `SceneGraph` / `MemoryAllocation` / `DebugMessage` の観察結果
11. 結論
12. 改善案

## 文書に入れてよい推測

- 初期 UI 生成が重い
- 画像読込が影響している
- Javascript の特定関数が主なロジックコストである

ただし、必ず観察根拠を先に示すこと。

## 避けること

- ソース対応を確認せずに行番号だけで断定すること
- Qt Creator と完全一致すると安易に書くこと
- raw event と集約表示の違いを無視すること
- 事実と解釈を混ぜて曖昧にすること

## 参照

- [QZT Evaluation Procedure](/Users/sugita/src/qt/tools/qmlprofileranalyzer/docs/SAMEGAME_QZT_PROCEDURE.md)
- [QZT Evaluation Work Instructions](/Users/sugita/src/qt/tools/qmlprofileranalyzer/docs/QZT_EVALUATION_WORK_INSTRUCTIONS.md)
