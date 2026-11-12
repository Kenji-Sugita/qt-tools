# qmlprofileranalyzer MCP Server Usage Notes

## 目的

`/usr/local/qt/qtmcpserver/` を使って `qmlprofileranalyzer` を MCP Server 化した場合に、AI から QML プロファイリング結果へどのような問い合わせができるようになるかを整理する。

現時点では構想メモであり、実装済み機能ではない。

## 基本方針

現在の `qmlprofileranalyzer` は CLI として次を提供している。

```bash
qmlprofileranalyzer summary trace.qzt
qmlprofileranalyzer hotspots trace.qzt
qmlprofileranalyzer ranges trace.qzt
qmlprofileranalyzer events-aggregated trace.qzt
qmlprofileranalyzer compare before.qzt after.qzt
qmlprofileranalyzer convert trace.qzt trace.qtd
qmlprofileranalyzer convert trace.qtd trace.qzt
```

MCP Server 化すると、これらを AI から直接呼び出せる tool として公開できる。
新しい解析能力を追加するというより、AI が必要な解析コマンドを自分で選んで実行し、対話的に深掘りできるようにすることが目的である。

## 想定する MCP tool

読み取り系:

- `qmlprofiler_summary`
- `qmlprofiler_hotspots`
- `qmlprofiler_ranges`
- `qmlprofiler_types`
- `qmlprofiler_events_aggregated`
- `qmlprofiler_notes`
- `qmlprofiler_compare`

ファイル作成系:

- `qmlprofiler_convert`

`convert` は出力ファイルを作成するため、`qtmcpserver` の `requiresApproval` を使って承認必須にするのがよい。

`events` と `ranges` は出力が非常に大きくなることがある。最初は `summary`, `hotspots`, `events-aggregated`, `compare` を中心に公開し、`ranges` は必要時に使う扱いが安全である。

## 問い合わせ例

### 全体把握

AI から次のような問い合わせができる。

- この trace の概要を教えて
- event 数、range 数、point event 数を教えて
- 一番長い range はどれか
- `Compiling` が目立つか
- note はあるか

対応 tool:

- `qmlprofiler_summary`
- `qmlprofiler_notes`

### 重い箇所の特定

- `selfTime` が大きい箇所を上から出して
- `inclusiveTime` が大きい箇所を教えて
- 呼び出し回数が多い binding を探して
- `burst` が出ている箇所を教えて
- `outlier` がある箇所を教えて
- 画像読み込みが関係しそうな箇所を探して

対応 tool:

- `qmlprofiler_hotspots`
- `qmlprofiler_events_aggregated`

### 詳細確認

- この hotspot の親子関係を確認して
- `selfTime` と `childTime` の内訳を見たい
- `gapWarning` が出ている range を確認して
- この file / line の range を詳しく見たい

対応 tool:

- `qmlprofiler_ranges`

`ranges` は出力が大きくなりやすいため、必要なときだけ呼ぶ。

### 種類別の確認

- Binding が多いか
- JavaScript が重いか
- `Compiling` がどれくらいあるか
- SceneGraph のイベントはどれくらいあるか
- PixmapCache や MemoryAllocation は目立つか

対応 tool:

- `qmlprofiler_types`
- `qmlprofiler_events_aggregated`
- `qmlprofiler_summary`

### 比較

- before と after で悪化した箇所はどこか
- 改善した hotspot はどれか
- 新しく出た hotspot はあるか
- 呼び出し回数が増えた箇所はあるか
- `medianSelf` が悪化した箇所はあるか

対応 tool:

- `qmlprofiler_compare`

### 形式変換

- この `.qzt` を `.qtd` に変換して
- この `.qtd` を `.qzt` にして共有しやすくして
- Qt Creator で開きやすい形式にして

対応 tool:

- `qmlprofiler_convert`

ファイル作成を伴うため承認必須にする。

## AI 側の利用フロー例

単一 trace の解析:

```text
1. qmlprofiler_summary で trace 全体を見る
2. qmlprofiler_hotspots で重い箇所を抽出する
3. 必要なら qmlprofiler_ranges で親子関係と selfTime を確認する
4. 結果を報告書としてまとめる
```

before / after 比較:

```text
1. qmlprofiler_compare で hotspot 差分を見る
2. 必要なら before / after それぞれに qmlprofiler_hotspots を実行する
3. 差分が大きい箇所だけ qmlprofiler_ranges で確認する
4. 悪化、改善、新規 hotspot を説明する
```

## 現在の運用との違い

現在は、人間または Codex が CLI を実行し、その出力を AI に渡して解析している。

MCP Server 化すると、AI が必要な tool を自分で選び、順に呼び出せる。

違い:

- CLI 出力を手で貼る必要が減る
- AI が `summary -> hotspots -> ranges` のように段階的に深掘りできる
- `tools/list` で利用可能な解析機能と引数 schema を AI に明示できる
- `convert` のようなファイル作成系操作を承認必須にできる
- 監査イベントで AI が何を呼んだか記録できる

## 実装構成案

既存 CLI を壊さないため、最初は別実行ファイルにするのが分かりやすい。

```text
qmlprofileranalyzer       既存 CLI
qmlprofileranalyzer-mcp   MCP Server
```

`qmlprofileranalyzer-mcp` は `qtmcpserver` の `QMcpServer` を使い、`registerTool()` で解析 tool を登録する。

`qmlprofileranalyzer` 側の既存クラスはそのまま利用できる。

- `QtdReader`
- `Analyzer`
- `Report`

最初の公開範囲:

- `summary`
- `hotspots`
- `events-aggregated`
- `compare`
- `notes`

必要に応じて追加:

- `ranges`
- `types`
- `convert`

## 注意点

- MCP Server 化は便利だが、現行 CLI の解析能力を置き換えるものではない。
- `ranges` と `events` は出力量が大きくなるため、初期公開範囲は慎重に決める。
- ファイル作成を伴う `convert` は承認必須にする。
- 公開する tool は allowlist で定義し、内部実装や不要なファイルパスを見せない。
- 実装前に、既存 CLI の仕様を変えない構成にする。
