# Implementation Plan

## 目的

QML プロファイラートレースを読み取り、内容を解析しやすい内部表現へ変換する CLI ツールを実装する。

初版では実装負荷と検証容易性を優先し、`.qtd` / Qt CLI 相当 XML を対象にする。

## スコープ

### 初版で対応するもの

- `.qtd` / XML の読み込み
- `trace`, `eventData`, `profilerDataModel`, `noteData` の解釈
- event type 一覧の構築
- range / 単発 event の復元
- note の読み込み
- テキストベースの解析出力

### 初版で対応しないもの

- `.qzt` の読み込み
- `.qtd` / `.qzt` の書き出し
- GUI
- 高度なフィルタや可視化

## 実装方針

### 基本方針

- まず `.qtd` を正しく読めるようにする
- Qt Creator の XML reader 実装に意味合わせする
- 内部モデルは `.qzt` reader を後から追加しやすい形にする
- 出力は当面 CLI 向けの summary と一覧に絞る

### 段階的実装

1. データモデル定義
2. XML reader 実装
3. summary/types/events/notes の CLI 実装
4. テストデータ整備
5. `.qzt` reader の追加

## 想定ファイル構成

- `CMakeLists.txt`
- `src/main.cpp`
- `src/model.h`
- `src/qtdreader.h`
- `src/qtdreader.cpp`
- `src/analyzer.h`
- `src/analyzer.cpp`
- `src/report.h`
- `src/report.cpp`
- `tests/`
- `samples/`

## モジュール設計

### `model`

責務:

- トレースの内部表現を定義する

主要構造:

- `TraceData`
- `EventTypeDef`
- `TraceEvent`
- `TraceNote`

保持したい情報:

- trace version
- trace start / end
- measured time
- event type table
- event list
- note list

### `QtdReader`

責務:

- XML を読み、`TraceData` を構築する

主な処理:

- `trace` 属性の取得
- `eventData/event` の読み込み
- `profilerDataModel/range` の読み込み
- `noteData/note` の読み込み

注意点:

- `duration` があるものは range として復元する
- `duration` がないものは単発 event として扱う
- `eventIndex` を event type table に関連付ける
- Qt Creator と同等に未知要素へ過剰反応しない

### `Analyzer`

責務:

- `TraceData` から基本統計を算出する

初版の集計:

- 総イベント数
- event type 数
- note 数
- 種別別件数
- 種別別累積時間
- 最長 range
- ファイル / 行ごとの件数

### `Report`

責務:

- 解析結果を CLI 向けテキストへ整形する

初版コマンド:

- `summary`
- `types`
- `events`
- `notes`

## CLI 案

```bash
qmlprofileranalyzer summary trace.qtd
qmlprofileranalyzer types trace.qtd
qmlprofileranalyzer events trace.qtd
qmlprofileranalyzer notes trace.qtd
qmlprofileranalyzer trace.qtd
```

補足:

- サブコマンドなしは `summary` 扱いにする案

## テストデータ計画

### 先に用意するもの

1. 最小手書き `.qtd`
2. range / event / note を含む手書き `.qtd`
3. 実行環境で採取した実トレース `.qtd`

### 後で用意するもの

- `.qzt` サンプル
- 異常系サンプル
  - 欠けた `eventIndex`
  - 未知 `type`
  - 壊れた XML

## `.qzt` 対応計画

### 前提

Qt Creator 実装では `.qzt` は次の形式:

- `QDataStream`
- magic: `QMLPROFILER`
- data stream version
- `traceStart`, `traceEnd`
- event types block
- notes block
- event blocks
- 各 block は `qCompress()` 済み

### `.qzt` 対応のために必要なもの

- `QmlEventType` 直列化形式の再現
- `QmlEvent` 直列化形式の再現
- `QmlNote` 直列化形式の再現
- block 単位の展開処理

### 方針

- `.qtd` reader 完成後に追加する
- `.qtd` と同じ `TraceData` へ流し込む

## リスク

- `.qzt` の直列化互換は Qt バージョン差の影響を受ける可能性がある
- XML は比較的安定だが、message / type の追加には追従が必要
- 実トレースには巨大ファイルがありうるため、将来的には streaming を考慮する

## 直近の実装順

1. `src/model.h`
2. `src/qtdreader.*`
3. `src/report.*`
4. `src/analyzer.*`
5. `src/main.cpp`
6. `CMakeLists.txt`
7. `tests` と `samples`
