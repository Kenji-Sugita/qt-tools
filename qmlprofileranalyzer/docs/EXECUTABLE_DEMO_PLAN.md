# Executable Demo Plan

## 目的

`qml` ベースの簡易デモとは別に、実行形式に近いプロファイリングデモを追加する。

## 採用例

- Qt Examples `linguist/localizedclock`

## 方針

- example を直接外部ビルドするのではなく、最小構成で `qmlprofileranalyzer` 配下に取り込む
- 自動終了タイマーを追加してトレース採取を安定化する
- `qmlprofiler --attach` で `.qtd` を生成する

## 追加物

- `demos/localizedclock/main.cpp`
- `demos/localizedclock/Main.qml`
- `scripts/generate_executable_demo_trace.sh`

## 期待する利点

- `qml` ランタイム固有の `default.qml` 系イベントとの差分が観察できる
- `.exe` / `a.out` に近い手順で demo trace を生成できる
