# Executable Demo Procedure

## 目的

この文書は、実行形式デモ `localizedclockdemo` を使ってトレースを生成し、
`docs/EXECUTABLE_DEMO_OUTPUTS.md` と
`docs/EXECUTABLE_DEMO_EVALUATION.md` に相当する結果を再現するための標準手順をまとめたもの。

## 前提条件

### 作業ディレクトリー

以後のコマンドは次のディレクトリーで実行する。

```bash
cd /Users/sugita/src/qt/tools/qmlprofileranalyzer
```

### 必要なコマンド

- `cmake`
- `qmlprofiler`
- `build/localizedclockdemo`

### 既知の前提

- `localizedclockdemo` は `QGuiApplication + QQmlApplicationEngine` の実行形式デモ
- `localizedclockdemo` は約15秒で自動終了する
- `-qmljsdebugger=...` を付けて起動し、`qmlprofiler --attach` で採取する
- sandbox や権限制約下では attach が失敗する場合がある
- 必要に応じて sandbox 外で実行する

## 手順 1. ビルド

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos
cmake --build build
```

生成される実行ファイル:

- `build/qmlprofileranalyzer`
- `build/localizedclockdemo`

## 手順 2. トレース生成

```bash
scripts/generate_executable_demo_trace.sh
```

既定の生成先:

- `samples/generated/localizedclockdemo.qtd`
- 完了まで約15秒かかる

## 手順 3. 解析

### summary

```bash
./build/qmlprofileranalyzer summary samples/generated/localizedclockdemo.qtd
```

### types

```bash
./build/qmlprofileranalyzer types samples/generated/localizedclockdemo.qtd | sed -n '1,30p'
```

### events

```bash
./build/qmlprofileranalyzer events samples/generated/localizedclockdemo.qtd | sed -n '1,160p'
```

## 手順 4. 観察ポイント

### 4.1 全体量

`summary` から次を確認する。

- `eventTypes`
- `events`
- `ranges`
- `pointEvents`
- `notes`

今回の観察:

- `eventTypes=37`
- `events=441`
- `ranges=106`
- `pointEvents=335`
- `notes=0`

### 4.2 最長 range

`summary` から次を確認する。

- `longestRange`

今回の観察:

- `idx=13`
- `type=Main.qml:43`
- `duration=472956334`

### 4.3 どのコード行か

対応ソース:

- `demos/localizedclock/Main.qml`

今回の観察:

- `Main.qml:43`
- `Text { text: qsTr("Locale: %1").arg(Qt.locale().name) }`

### 4.4 繰り返し更新

`summary` の per-type から次を確認する。

- `Main.qml:56`
- `Main.qml:32`

今回の観察:

- `Main.qml:56`
  - `count=15`
  - `totalDuration=82430122`
- `Main.qml:32`
  - `count=16`
  - `totalDuration=1747374`

### 4.5 主要 type 群

`types` の先頭から次を確認する。

- `Compiling`
- `Creating`
- `Binding`
- `Javascript`
- `MemoryAllocation`
- `SceneGraph`
- `AnimationFrame`
- `DebugMessage`

## 手順 5. 評価へ落とす観点

`docs/EXECUTABLE_DEMO_EVALUATION.md` を作るときは、少なくとも次を含める。

1. 実行形式デモとして妥当か
2. 最長 range とそのコード位置
3. binding / javascript / timer 更新がどう見えているか
4. `qml` デモより実運用に近い点
5. フォント警告など環境ノイズの扱い

## 既知の注意点

### 1. 実行デモは環境依存

- attach が失敗する場合がある
- 接続できない場合は、権限、ポート、sandbox 制約を疑う

### 2. QML debugging 有効ビルドが必要

- `localizedclockdemo` は `QT_QML_DEBUG` を付けてビルドしている
- これが無いと `-qmljsdebugger=...` が無視される

### 3. フォント警告

- `Text` の duration に影響している可能性がある

## 関連文書

- `docs/EXECUTABLE_DEMO_OUTPUTS.md`
- `docs/EXECUTABLE_DEMO_RESULTS.md`
- `docs/EXECUTABLE_DEMO_EVALUATION.md`
- `docs/EXECUTABLE_DEMO_PLAN.md`
