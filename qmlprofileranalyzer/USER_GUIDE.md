---
genpdf:
  format: book
  title: qmlprofileranalyzer 利用ガイド
  subtitle: 現行版
  author: (株) SRA
  page_numbers: true
---

# 1. 概要

`qmlprofileranalyzer` は、Qt の QML Profiler トレースを読み取り、内容をテキストで確認するためのコマンドラインツールです。

現在の `qmlprofileranalyzer` は、次の用途に向いています。

- `.qtd` / XML トレースを読む
- `.qzt` / binary トレースを読む
- QML Profiler の概要、type、event、note を確認する
- range の親子関係と `selfTime` を確認する
- hotspot, 外れ値, バースト, 画像影響候補, `Compiling` の注意情報を確認する
- Qt Creator の表示と比較しやすい補助情報を出す
- 調査結果や評価文書の根拠を取り出す

現時点で対応している入力形式は次です。

- `.qtd`
- `.qzt`

CLI の help と usage では、入力ファイルを `<trace.qtd|trace.qzt>` と表記します。`.qtd` は XML 形式、`.qzt` は Qt Creator の binary 形式です。

現時点での出力はテキストのみです。JSON や CSV はありません。

# 2. 基本的な考え方

`qmlprofileranalyzer` は、トレースファイルを読み込んで、指定した観点ごとに表示します。`compare` 以外は 1 ファイル、`compare` は 2 ファイルを入力します。

- `summary`
  - 全体統計を表示します
- `ranges`
  - range の親子関係、`selfTime`、`gapWarning`、`focus` を表示します
- `hotspots`
  - `selfTime` ベースの重い箇所を集約して表示します
- `compare`
  - 2 つのトレースの `hotspots` を `groupKey` で比較します
- `convert`
  - `.qtd` と `.qzt` を相互変換します
- `types`
  - event type 一覧を表示します
- `events`
  - raw event をそのまま表示します
- `events-aggregated`
  - raw event に加えて、Qt Creator 比較向けの集約行を表示します
- `notes`
  - note を表示します

引数を 1 つだけ渡した場合は、`summary` として扱います。

```text
$ qmlprofileranalyzer trace.qzt
```

これは次と同じ意味です。

```text
$ qmlprofileranalyzer summary trace.qzt
```

# 3. ビルド

Qt 6.11.0 を前提にビルドする例は次です。

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos
cmake --build build
ctest --test-dir build --output-on-failure
```

環境によって Qt license service の警告が出る場合があります。テスト実行時に必要であれば、次のように `QTFRAMEWORK_BYPASS_LICENSE_CHECK=1` を付けます。

```bash
QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 ctest --test-dir build --output-on-failure
```

生成される実行ファイルは次です。

```text
./build/qmlprofileranalyzer
```

# 4. 起動

通常は次のように起動します。

```text
$ ./build/qmlprofileranalyzer summary samples/minimal.qtd
```

最小形は次です。

```text
$ ./build/qmlprofileranalyzer samples/minimal.qtd
```

ヘルプは次のように表示します。

```text
$ ./build/qmlprofileranalyzer --help
```

# 5. 初見の人向けの最短手順

初見の場合は、まず次の 3 コマンドだけを使います。

## 5.1 全体を見る

```text
$ ./build/qmlprofileranalyzer summary trace.qzt
```

ここでは次を見ます。

- トレース全体の規模
- `ranges`
- `pointEvents`
- `longestRange`
- `Compiling` の有無

注意:

- `summary` の `totalDuration` は親子重複を含みます
- `Creating` / `Binding` の重さは `summary` だけで判断しません

## 5.2 重い箇所を見る

```text
$ ./build/qmlprofileranalyzer hotspots trace.qzt
```

ここでは次を見ます。

- `selfTotal`
- `inclusiveTotal`
- `calls`
- `medianSelf`
- `maxSelf`
- `outlier`
- `burst`
- `image`
- `compilingNote`

`hotspots` は重い箇所を絞って見たいときの主力コマンドです。

## 5.3 詳細を掘る

```text
$ ./build/qmlprofileranalyzer ranges trace.qzt
```

ここでは次を見ます。

- `selfTime`
- `childTime`
- `gapTime`
- `gapWarning`
- `focus`
- `depth`
- `parent`

`ranges` は親子関係を見たいときの主力コマンドです。

初見の人向けの使い方は、まず `summary -> hotspots -> ranges` の順で十分です。
`types`, `events`, `events-aggregated` は必要になってから使います。

# 6. 最初に試す例

## 6.1 最小 `.qtd` を読む

```text
$ ./build/qmlprofileranalyzer summary samples/minimal.qtd
```

ここでは次のような情報を確認できます。

- `traceStart`
- `traceEnd`
- `traceDuration`
- `eventTypes`
- `events`
- `ranges`
- `pointEvents`
- `notes`
- `longestRange`

## 6.2 type 一覧を確認する

```text
$ ./build/qmlprofileranalyzer types samples/minimal.qtd
```

ここでは次のような情報を確認できます。

- type 名
- display 名
- file / line / column
- detail 情報
- details

## 6.3 range を確認する

```text
$ ./build/qmlprofileranalyzer ranges tests/data/samegame.qzt
```

ここでは次のような情報を確認できます。

- `duration`
- `selfTime`
- `childTime`
- `gapTime`
- `gapWarning`
- `focus`
- `depth`
- `parent`

## 6.4 hotspot を確認する

```text
$ ./build/qmlprofileranalyzer hotspots tests/data/samegame.qzt
```

ここでは次のような情報を確認できます。

- `selfTotal`
- `inclusiveTotal`
- `groupKey`
- `indexes`
- `calls`
- `minSelf`
- `medianSelf`
- `maxSelf`
- `outlier`
- `burst`
- `image`
- `compilingNote`

## 6.5 raw event を確認する

```text
$ ./build/qmlprofileranalyzer events samples/minimal.qtd
```

`events` は raw event 専用です。既存の event 並びをそのまま見たい場合に使います。

## 6.6 集約付き event を確認する

```text
$ ./build/qmlprofileranalyzer events-aggregated tests/data/samegame.qzt
```

`events-aggregated` では、先頭に `# aggregates` 節が追加されます。

現在は次の対象を補助的に集約します。

- `PixmapCache`
- `SceneGraph`
- `MemoryAllocation`
- `DebugMessage`

また、Qt Creator と比較しやすいように、次の補助フィールドを付けます。

- `qtCreatorCategory`
- `qtCreatorLabel`
- `qtCreatorType`
- `qtCreatorThread`
- `qtCreatorStages`

# 7. コマンド一覧

## 7.1 `summary`

```text
$ ./build/qmlprofileranalyzer summary trace.qzt
```

全体統計を表示します。

向いている用途:

- トレースの規模確認
- 最長 range の確認
- 最初の当たりを付ける
- `Compiling` の有無を確認する

注意:

- `totalDuration` は親子重複を含みます
- 詳細分析は `hotspots` と `ranges` で行います

## 7.2 `ranges`

```text
$ ./build/qmlprofileranalyzer ranges trace.qzt
```

range の親子関係と `selfTime` を表示します。

向いている用途:

- 親子重複の確認
- 真に重い leaf の確認
- `gapWarning` の確認
- `focus` 候補の確認

主なフィールド:

- `duration`: range 全体の時間
- `selfTime`: 子 range を除いた時間
- `childTime`: 子 range が覆っている時間
- `gapTime`: 現時点では `selfTime` と同じ値
- `gapWarning`: QML 子 range では説明しにくい時間の候補
- `focus`: 子孫より自己時間が目立つ候補

## 7.3 `hotspots`

```text
$ ./build/qmlprofileranalyzer hotspots trace.qzt
```

重い箇所を集約して表示します。

向いている用途:

- `selfTime` ベースで重い箇所を絞る
- 外れ値の確認
- バースト候補の確認
- 画像影響候補の確認
- `Compiling` 注意情報の確認

主なフィールド:

- `calls`: 呼び出し回数
- `groupKey`: 複数トレース比較向けの安定した集約キー
- `indexes`: この hotspot に統合された event index 一覧
- `selfTotal`: 自己時間の合計
- `inclusiveTotal`: 親子を含む時間の合計
- `medianSelf`: 自己時間の中央値
- `maxSelf`: 自己時間の最大値
- `outlier`: 外れ値候補
- `burst`: 短時間に集中して発生した候補
- `image`: 画像ロードと重なった候補
- `compilingNote`: `Compiling` 解釈への注意

## 7.4 `compare`

```text
$ ./build/qmlprofileranalyzer compare before.qzt after.qzt
```

2 つのトレースの `hotspots` を `groupKey` で突き合わせ、差分を表示します。

向いている用途:

- 同じ操作を複数回測定したときのばらつき確認
- 変更前後で重くなった箇所、軽くなった箇所の確認
- `eventIndex` が異なるトレース同士の比較

主なフィールド:

- `status`: `added`, `removed`, `changed`, `unchanged`
- `groupKey`: 比較に使った hotspot 集約キー
- `beforeSelfTotal` / `afterSelfTotal`: 比較前後の自己時間合計
- `selfDelta` / `selfDeltaPct`: 自己時間合計の差分
- `beforeCalls` / `afterCalls`: 呼び出し回数
- `callsDelta`: 呼び出し回数の差分
- `medianSelfDelta`: 自己時間中央値の差分
- `maxSelfDelta`: 自己時間最大値の差分

`compare` は `hotspots` の比較に限定しています。親子関係や `gapWarning` の詳細確認は、差分が大きい `groupKey` を見つけてから各トレースの `ranges` で確認します。

## 7.5 `types`

```text
$ ./build/qmlprofileranalyzer types trace.qzt
```

type 情報を表示します。

向いている用途:

- どのコード位置が記録されているか調べる
- `Creating`, `Binding`, `Javascript`, `Compiling` などを確認する
- `eventIndex` と file / line を対応付ける

## 7.6 `events`

```text
$ ./build/qmlprofileranalyzer events trace.qzt
```

raw event を表示します。

向いている用途:

- 生データを確認する
- 集約前のイベント列を追う
- `events-aggregated` の補助集約を検証する

## 7.7 `events-aggregated`

```text
$ ./build/qmlprofileranalyzer events-aggregated trace.qzt
```

集約行と raw event をまとめて表示します。

向いている用途:

- Qt Creator 比較
- `PixmapCache` の読込時間確認
- `SceneGraph` の frame 観察
- `MemoryAllocation` の種別 summary 確認
- `DebugMessage` の level 別確認

## 7.8 `notes`

```text
$ ./build/qmlprofileranalyzer notes trace.qtd
```

note を表示します。

向いている用途:

- `noteData` の確認
- 解析メモの確認

# 8. `.qtd` と `.qzt` の違い

`qmlprofileranalyzer` は両方を読めますが、意味は少し違います。

- `.qtd`
  - XML
  - CLI `qmlprofiler` の出力に近い
- `.qzt`
  - binary
  - Qt Creator の既定保存形式

Qt Creator と直接比較したい場合は、`.qzt` の方が自然です。

`.qzt` をテキストとして確認したい場合は、`convert` で `.qtd` XML に変換できます。`.qtd` を保存・共有しやすい binary 形式にしたい場合は、`.qzt` に変換できます。

```bash
./build/qmlprofileranalyzer convert trace.qzt trace.qtd
./build/qmlprofileranalyzer convert trace.qtd trace.qzt
```

`convert` は拡張子で変換方向を判断します。対応している組み合わせは `.qzt -> .qtd` と `.qtd -> .qzt` です。

注意点として、`.qzt` 形式は `.qtd` の `eventData totalTime` を直接持たないため、`.qtd -> .qzt` 後に summary すると `totalTime` は `traceEnd - traceStart` として扱われます。

# 9. 解析の進め方

通常は次の順で見ると分かりやすいです。

1. `summary`
2. `hotspots`
3. `ranges`
4. `types`
5. 必要なら `events-aggregated`
6. 必要なら `events`

補足:

- `summary` は初動向けです
- 詳細分析は `hotspots` と `ranges` が中心です
- `Creating` / `Binding` は親子関係を見ないと誤誘導しやすいです
- Qt Creator と比較したい場合は `events-aggregated` を使います

# 10. 比較について

初回トレースと再訪トレースの比較は、`compare` コマンドで `hotspots` 差分を確認できます。
詳細な原因確認は、比較後に単一トレース用の `hotspots` と `ranges` で掘ります。

参照:

- `docs/TRACE_COMPARISON_GUIDE.md`

方針:

- 比較前に測定条件を揃える
- まず各トレースを単独で読む
- `compare before.qzt after.qzt` で `hotspots` 差分を見る
- 比較では `summary` より `hotspots` と `ranges` を重視する
- `eventIndex` はトレース間で安定しない可能性があるため、`hotspots` では `groupKey` を優先して見る
- 必要に応じて `display`, `file`, `line`, `type` も併用する

# 11. Qt Creator と比較するときの見方

Qt Creator と `qmlprofileranalyzer` は、表示モデルが完全には同じではありません。

ただし `events-aggregated` により、比較はしやすくなっています。

主な対応関係は次です。

- `PixmapCacheLoad`
  - Qt Creator の `Image Loaded` 比較に向く
- `SceneGraphFrame`
  - Qt Creator の scene graph タイムライン比較に向く
- `MemoryAllocationSummary`
  - Qt Creator の memory usage 比較に向く
- `DebugMessageSummary`
  - Qt Creator の debug messages 比較に向く

Qt Creator と比較したい場合は、まず次の順で見るのが分かりやすいです。

1. `summary`
2. `hotspots`
3. `ranges`
4. `events-aggregated`
5. 必要なら `events`

# 12. デモ

## 12.1 最小デモ

固定サンプルは次です。

```text
samples/minimal.qtd
```

試す例:

```text
$ ./build/qmlprofileranalyzer summary samples/minimal.qtd
$ ./build/qmlprofileranalyzer types samples/minimal.qtd
$ ./build/qmlprofileranalyzer notes samples/minimal.qtd
```

## 12.2 `qml` ベース実行デモ

```text
$ scripts/generate_demo_trace.sh
$ ./build/qmlprofileranalyzer summary samples/generated/autotrace.qtd
```

補足:

- `qml` + `qmlprofiler --attach` を使います
- `qmlscene` は使いません
- `qml` ランタイム由来の `default.qml` 系イベントが入る場合があります

## 12.3 実行形式デモ

```text
$ scripts/generate_executable_demo_trace.sh
$ ./build/qmlprofileranalyzer summary samples/generated/localizedclockdemo.qtd
```

補足:

- `localizedclockdemo` を起動して attach します
- 実運用に近い確認用です
- `demos/localizedclock/main.cpp` が存在する場合だけ、CMake で `localizedclockdemo` をビルドします

## 12.4 サンプル解析報告書

配布物には、サンプルトレースを解析した報告書を含めています。

- `samples/SAMPLES_TRACE_REPORT.md`
- `samples/SAMPLES_TRACE_REPORT.pdf`

この報告書は、次のトレースの解析例として参照できます。

- `samples/minimal.qtd`
- `samples/generated/autotrace.qtd`
- `samples/generated/localizedclockdemo.qtd`

# 13. 評価文書を作るときの基本手順

`.qzt` と対応ソースコードがある場合は、次の順で進めます。

1. `summary` で全体量を見る
2. `hotspots` で重い箇所を絞る
3. `ranges` で親子関係と `selfTime` を確認する
4. `types` でコード位置と type を確認する
5. `events-aggregated` で比較しやすい集約を見る
6. 必要なら `events` で raw event を確認する
7. ソースコードへ行番号を対応付ける

関連文書:

- `docs/SAMEGAME_QZT_PROCEDURE.md`
- `docs/QZT_EVALUATION_WORK_INSTRUCTIONS.md`
- `docs/QZT_EVALUATION_AGENT_INSTRUCTIONS.md`

# 14. 利用上の注意

- `compare` 以外の入力は 1 ファイルずつ扱います
- 出力は現時点ではテキストだけです
- 例外として、`convert` は `.qtd` / `.qzt` のトレースファイルを書き出します
- `events-aggregated` の集約行は補助情報です
- Qt Creator と完全に同じ UI や分類名を再現しているわけではありません
- `Compiling` は QML コンパイルだけでなく import や plugin load を含む可能性があります
- `gapWarning` は原因断定ではなく、QML 子 range では説明しにくい時間の候補です

# 15. 現時点で未対応のもの

次は現時点で未対応です。

- JSON 出力
- CSV 出力
- 機械可読な structured output
- GUI
- 時間範囲フィルタ
- type / file / line フィルタ
- 上位 N 件表示
- file / line 単位の詳細統計
- caller / callee 解析
- flame graph 相当の集計
- 大容量ファイル向け最適化

複数トレース比較は `compare` で `hotspots` 差分のみ対応しています。`ranges` 比較や統計的な有意差判定は未対応です。

詳しくは次を参照します。

- `docs/STATUS.md`
- `docs/ROADMAP.md`
