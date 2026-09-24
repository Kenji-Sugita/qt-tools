# Demo Results

## 対象

この文書は、`qmlprofileranalyzer` に含まれている 2 種類のデモ結果を要約する。

- 最小デモ
  - 入力: `samples/minimal.qtd`
- 実行デモ
  - 入力: `samples/generated/autotrace.qtd`
  - 元 QML: `samples/qml/AutoTrace.qml`

詳細なコマンド出力全文は `docs/DEMO_OUTPUTS.md` を参照。

## 1. 最小デモ

### 入力

- 固定 XML トレース
- 種別数 3
- event 数 3
- note 数 1

### 確認できたこと

- `.qtd` / XML を正常に読める
- `summary` が trace 全体を要約できる
- `types` が `Binding`, `Event`, `DebugMessage` を列挙できる
- `notes` が note を読める
- `duration` を持つ event を range として扱えている

### 結果の要点

- `version=1.02`
- `traceStart=1000`
- `traceEnd=1800`
- `traceDuration=800`
- `eventTypes=3`
- `events=3`
- `ranges=1`
- `pointEvents=2`
- `notes=1`
- 最長 range
  - `root width binding`
  - `duration=40`

## 2. 実行デモ

### 入力

- `samples/qml/AutoTrace.qml` を `qml` と `qmlprofiler` で実行して採取
- 出力トレース: `samples/generated/autotrace.qtd`

現在の標準実行基準:

- `qml` + `qmlprofiler --attach`

### 確認できたこと

- 実トレースを生成できる
- 生成した `.qtd` を解析ツールで読める
- 最小サンプルだけでなく、実環境由来の event 群にも対応できている
- `Compiling`, `Creating`, `Binding`, `Javascript`, `MemoryAllocation`, `SceneGraph`,
  `AnimationFrame`, `DebugMessage` を含むトレースを読める
- `qml` ランタイム由来の `default.qml` 系 event も読める

### 結果の要点

- `version=1.02`
- `eventTypes=26`
- `events=423`
- `ranges=24`
- `pointEvents=399`
- `notes=0`
- 最長 range
  - `AutoTrace.qml:35`
  - `duration=508066625`

### 読み取れること

- QML object creation と binding 評価が取れている
- scene graph と animation frame の event も含まれている
- memory allocation event も拾えている
- `qml` の runtime config 由来で `default.qml:*` の event が追加されている
- 初版 reader は、固定サンプルだけでなく現物トレースにも通用している

## 3. 現時点の結論

- 初版の `.qtd` reader は動作している
- 最小デモと実行デモの両方で動作確認済み
- 次の優先課題は `.qzt` reader と自動テスト追加
