# Demo Procedure

## 目的

この文書は、`qmlprofileranalyzer` のデモを再実行し、
`docs/DEMO_OUTPUTS.md` と `docs/DEMO_EVALUATION.md` に相当する結果を
再現するための標準手順をまとめたもの。

この手順で再現したいもの:

- 最小デモの解析結果
- 実行デモのトレース生成と解析結果
- 実行デモのプロファイリング結果に対する評価

## 前提条件

### 作業ディレクトリー

以後のコマンドは次のディレクトリーで実行する。

```bash
cd /Users/sugita/src/qt/tools/qmlprofileranalyzer
```

この前提で、本文中の相対パスを解釈する。

### 必要なコマンド

- `cmake`
- `qmlprofiler`
- `qml`

既定の参照先:

- `/usr/local/qt/Qt/6.11.0/macos/bin/qmlprofiler`
- `/usr/local/qt/Qt/6.11.0/macos/bin/qml`

### 環境依存

実行デモは `qml` と `qmlprofiler --attach` を使うため、環境によっては
接続待ちやポート待受けで詰まることがある。

注意点:

- フォント警告が出ることがある
- sandbox や権限制約下では実行デモが通らない場合がある
- 必要に応じて権限を上げて実行確認する
- `qml` ランタイム由来の `default.qml` 系 event がトレースに追加される場合がある

## 手順 1. ビルド

### configure

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos
```

### build

```bash
cmake --build build
```

### 確認

生成される実行ファイル:

- `build/qmlprofileranalyzer`

## 手順 2. 最小デモ

### 2.1 summary

```bash
./build/qmlprofileranalyzer summary samples/minimal.qtd
```

### 2.2 types

```bash
./build/qmlprofileranalyzer types samples/minimal.qtd
```

### 2.3 notes

```bash
./build/qmlprofileranalyzer notes samples/minimal.qtd
```

### 最小デモで確認すること

- `.qtd` / XML が読める
- `trace`, `eventData`, `profilerDataModel`, `noteData` を解釈できる
- `Binding`, `Event`, `DebugMessage` が読める
- note を 1 件読める

### 期待される観察点

- `eventTypes=3`
- `events=3`
- `ranges=1`
- `notes=1`
- 最長 range は `root width binding`

対応する保存済み出力:

- `docs/DEMO_OUTPUTS.md`

## 手順 3. 実行デモ

### 3.1 トレース生成

```bash
scripts/generate_demo_trace.sh
```

既定の生成先:

- `samples/generated/autotrace.qtd`

### 3.2 summary

```bash
./build/qmlprofileranalyzer summary samples/generated/autotrace.qtd
```

### 3.3 types

```bash
./build/qmlprofileranalyzer types samples/generated/autotrace.qtd | sed -n '1,20p'
```

### 実行デモで確認すること

- 実 QML から `.qtd` トレースを生成できる
- 生成トレースを `qmlprofileranalyzer` で読める
- `Compiling`, `Creating`, `Binding`, `Javascript`, `MemoryAllocation`,
  `SceneGraph`, `AnimationFrame`, `DebugMessage` が含まれている

### 期待される観察点

- `eventTypes=26`
- `events=423`
- `ranges=24`
- `pointEvents=399`
- `notes=0`
- 最長 range は `AutoTrace.qml:35`
- `default.qml:*` の event が追加で見える

対応する保存済み出力:

- `docs/DEMO_OUTPUTS.md`

## 手順 4. 評価の作り方

`docs/DEMO_EVALUATION.md` は、主に次の観察に基づいて作成する。

### 4.1 最長 range の確認

使用する出力:

- `summary samples/generated/autotrace.qtd`

見る項目:

- `longestRange`

今回の観察:

- `idx=12`
- `type=AutoTrace.qml:35`
- `duration=508066625`

### 4.2 どのコード行かを確認

使用するファイル:

- `samples/qml/AutoTrace.qml`

見る項目:

- `35` 行目

今回の観察:

- `Text` 要素

### 4.3 次に大きい処理を確認

使用する出力:

- `summary samples/generated/autotrace.qtd`
- 必要に応じて `events samples/generated/autotrace.qtd`

見る項目:

- `per-type`
- 大きい `totalDuration`
- 大きい `maxDuration`

今回の観察例:

- `AutoTrace.qml:0`
  - `Compiling` 相当
- `AutoTrace.qml:4`
  - `Window`
- `AutoTrace.qml:17`
  - `Rectangle`
- `AutoTrace.qml:26`
  - `NumberAnimation`
- `AutoTrace.qml:43`
  - `Timer`

### 4.4 警告を環境要因として扱う

トレース生成時の出力で見る項目:

- font family alias 警告

解釈:

- これらは純粋な QML ロジックだけの負荷ではない可能性を示す
- 特に `Text` の大きい duration は、フォントや初回描画初期化の影響を
  含む可能性がある

## 手順 5. `DEMO_EVALUATION.md` へ反映する観点

評価文では、少なくとも次を含める。

1. 全体評価
- デモが軽量か
- イベントの多くが補助イベントか

2. 主要な観察点
- 最長 range
- 次に大きい処理

3. 解釈
- 主要イベントを取得できているか
- そのまま性能評価に使いにくい要因があるか

4. コード単位の見立て
- 軽い箇所
- 相対的に重い箇所

5. 結論
- 動作確認用デモとして十分か
- 性能ベンチマーク用として十分か

## 既知の注意点

### 1. 実行デモは環境依存

- 環境によって `generate_demo_trace.sh` が待ち状態になる可能性がある
- 接続できない場合は、権限、ポート、sandbox 制約を疑う

### 2. `qml` ランタイム由来の追加イベント

- `default.qml` 系の `Compiling` / `Creating` が混ざる場合がある
- そのため、過去の `qmlscene` ベース結果と件数が一致しないことがある

### 3. フォント警告

- `Text` の結果に影響している可能性がある

### 4. 出力値は環境差を受ける

- duration や event 数は環境によって多少変わりうる
- 評価では傾向を見る

## 関連文書

- `docs/DEMO_OUTPUTS.md`
- `docs/DEMO_EVALUATION.md`
- `samples/README.md`
- `README.md`
