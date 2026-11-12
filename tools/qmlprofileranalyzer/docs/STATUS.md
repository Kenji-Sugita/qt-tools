# Status

## 実装済み

### コア機能

- `.qtd` / XML トレース読み込み
- `.qzt` / binary トレース読み込み
- `trace` ルート属性の読み込み
  - `version`
  - `traceStart`
  - `traceEnd`
- `eventData/event` の読み込み
- `profilerDataModel/range` の読み込み
- `noteData/note` の読み込み
- event type table の構築
- range と単発 event の区別
- 必須属性が欠けた `event` / `range` / `note` の読み飛ばし
- `.qzt` の start/end event から range を再構成

### CLI

- `summary`
- `types`
- `events`
- `events-aggregated`
- `notes`
- `convert`
- サブコマンド省略時の `summary` 扱い
- `.qzt` の `PixmapCache`, `SceneGraph`, `MemoryAllocation`, `DebugMessage` を `events-aggregated` で集約表示
- `events-aggregated` の集約行に Qt Creator 比較用ラベルを付与
- `convert <input.qzt> <output.qtd>` による `.qzt` から `.qtd` XML への変換
- `convert <input.qtd> <output.qzt>` による `.qtd` から `.qzt` binary への変換

### 集計

- event type 数
- event 数
- range 数
- 単発 event 数
- note 数
- 種別別件数
- 種別別累積 duration
- 種別別最大 duration
- 最長 range

### デモ

- 固定最小デモ
  - `samples/minimal.qtd`
- 実行デモ
  - `samples/qml/AutoTrace.qml`
  - `scripts/generate_demo_trace.sh`
  - `qml` + `qmlprofiler --attach` を使用
- 実トレース生成確認済み
  - `samples/generated/autotrace.qtd`
  - `qml` ランタイム由来の `default.qml` 系 event を含む場合がある
- 実行形式デモ
  - `demos/localizedclock/Main.qml`
  - `build/localizedclockdemo`
  - `scripts/generate_executable_demo_trace.sh`
- 実行形式デモトレース生成確認済み
  - `samples/generated/localizedclockdemo.qtd`

### ドキュメント

- フォーマット調査メモ
- 出典一覧
- 実装計画
- エビデンス行番号
- 比較運用ガイド

### 品質

- `QTest` + `CTest` による自動テスト
- 正常系テストデータ
- 異常系テストデータ
- `.qzt` fixture テスト
- `samegame.qzt` を使った `PixmapCache` 集約表示テスト
- synthetic trace を使った `SceneGraph`, `MemoryAllocation`, `DebugMessage` 集約表示テスト

## 未実装

### 出力

- JSON 出力
- CSV 出力
- 機械可読な structured output

### 分析機能

- 時間範囲フィルタ
- type / file / line フィルタ
- 上位 N 件表示
- file / line 単位の詳細統計
- caller / callee 解析
- flame graph 相当の集計

### 優先実装の進捗

完了:

- range 親子関係復元
- `selfTime` / `gapTime` 算出
- `ranges` コマンド追加
- `focus` による着目候補表示
- `gapWarning` による `gapTime` 注意表示
- `summary` への親子重複注意表示
- `hotspots` コマンド追加
- `count` / `min` / `max` / `median` による hotspot 集約
- `file + line + column + type` 系の安定キーによる hotspot 集約
- `hotspots` への `groupKey` / `indexes` 表示
- `compare` コマンドによる複数トレースの hotspot 差分比較
- `convert` コマンドによる `.qzt -> .qtd` 変換
- `convert` コマンドによる `.qtd -> .qzt` 変換
- `outlier` ヒント
- `burst` ヒント
- 画像影響の補助表示
- `Compiling` 解釈への注意表示

未完了:

- なし

### 残作業

- 初回/再訪比較は機能実装ではなく運用ガイドで扱う
- `compare` は `hotspots` 差分比較に限定し、`ranges` の自動比較は未対応
- `compare` の出力形式は改善余地がある
- `compare` の上位 N 件表示、差分しきい値、`changed` のみ表示などの絞り込みは未対応
- `compare` の CSV / JSON など機械可読出力は未対応
- 外れ値判定の閾値設計は今後の調整余地がある
- バースト判定ルールは今後の調整余地がある

### 品質

- 大容量ファイル向け最適化

## 今回の実装範囲

今回の実装で意図した範囲:

- `.qtd` / XML reader
- `.qzt` / binary reader
- `.qtd` / `.qzt` converter
- CLI による可視化
- 最小デモ
- 実行デモ

今回の実装で意図的に外した範囲:

- writer を CLI の汎用保存コマンドとして公開すること
- GUI
- 高度な分析機能

## 確認状況

確認済み:

- CMake configure
- CMake build
- `ctest`
- `samples/minimal.qtd` の解析
- `qml` ベース実行デモでの `.qtd` 生成
- 生成された `.qtd` の解析
- 実行形式デモでの `.qtd` 生成
- 実行形式デモの `.qtd` 解析

未確認:

- Qt バージョン差異の吸収
