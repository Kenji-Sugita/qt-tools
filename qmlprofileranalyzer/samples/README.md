# Samples

## 最小デモ

固定サンプル:

- `minimal.qtd`

例:

```bash
qmlprofileranalyzer summary samples/minimal.qtd
qmlprofileranalyzer types samples/minimal.qtd
qmlprofileranalyzer events samples/minimal.qtd
qmlprofileranalyzer notes samples/minimal.qtd
```

## 実行デモ

QML サンプル:

- `qml/AutoTrace.qml`

トレース生成:

```bash
scripts/generate_demo_trace.sh
```

既定出力先:

- `samples/generated/autotrace.qtd`

補足:

- 実行デモは `qml` を使う
- `qml` ランタイムの初期化由来で `default.qml` 系の event が含まれる場合がある

## 実行形式デモ

生成スクリプト:

```bash
scripts/generate_executable_demo_trace.sh
```

既定出力先:

- `samples/generated/localizedclockdemo.qtd`

補足:

- `build/localizedclockdemo` を起動して採取する
- `localizedclockdemo` は約15秒で自動終了する
- `qml` デモより実運用に近い
