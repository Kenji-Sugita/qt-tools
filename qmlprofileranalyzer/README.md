# qmlprofiler-analyzer

QML プロファイラー解析ツール用の作業ディレクトリーです。

現時点で確認できている重要事項:

- Qt `qmlprofiler` CLI の保存形式は `.qtd` 相当の XML
- Qt Creator は `.qtd` と `.qzt` の両方を読み書きする
- `.qtd` は XML
- `.qzt` は `QDataStream` ベースのバイナリで、各ブロックを `qCompress()` して保存する
- Qt Creator の GUI 既定保存拡張子は `.qzt`

実装上の優先順位:

1. `.qtd` / CLI XML を読む
2. `.qzt` バイナリを読む
3. 必要なら Qt Creator 相互運用用の書き出しを実装する

現状の reader 対応:

- `.qtd` / XML
- `.qzt` / binary

CLI の入力引数も `.qtd` と `.qzt` の両方を受け付ける。`--help` では `<trace.qtd|trace.qzt>` と表記する。

構成:

- `docs/FORMAT.md`
  - フォーマットの要点
- `docs/SOURCES.md`
  - ローカル参照元、Qt Creator clone 情報、Web 出典
- `docs/IMPLEMENTATION_PLAN.md`
  - 実装順とスコープ
- `docs/TRACE_COMPARISON_GUIDE.md`
  - 初回/再訪トレース比較の運用ガイド
- `references/qtdeclarative/`
  - Qt 側 `qmlprofiler` 関連ソースのコピー
- `references/qtcreator/`
  - Qt Creator 側 trace reader/writer 関連ソースのコピー
- `samples/`
  - 最小デモ `.qtd` と実行デモ QML
- `scripts/generate_demo_trace.sh`
  - 実トレース採取用スクリプト
- `scripts/generate_executable_demo_trace.sh`
  - 実行形式デモ用トレース採取スクリプト
- `src/`
  - CLI 本体
- `demos/`
  - 実行形式デモ用ソース

ビルド:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos
cmake --build build
ctest --test-dir build --output-on-failure
```

お手軽使用方法:

初見の場合は、まず次の 3 コマンドだけで十分です。

1. 全体を見る

```bash
./build/qmlprofileranalyzer summary trace.qzt
```

2. 重い箇所を見る

```bash
./build/qmlprofileranalyzer hotspots trace.qzt
```

3. 詳細を掘る

```bash
./build/qmlprofileranalyzer ranges trace.qzt
```

2 つのトレースを比較する場合は次を使います。

```bash
./build/qmlprofileranalyzer compare before.qzt after.qzt
```

`.qtd` と `.qzt` を相互変換する場合は次を使います。

```bash
./build/qmlprofileranalyzer convert input.qzt output.qtd
./build/qmlprofileranalyzer convert input.qtd output.qzt
```

見方の目安:

- `summary`
  - トレース全体の規模を見る
- `hotspots`
  - どこが重いかを絞る
- `compare`
  - 2 つのトレースの `hotspots` を `groupKey` で比較する
- `convert`
  - `.qtd` と `.qzt` を相互変換する
- `ranges`
  - 親子関係や `selfTime` を確認する

補足:

- まずは `summary -> hotspots -> ranges` の順で見る
- `events` や `types` は、必要になったときだけ使えばよい

最小デモ:

```bash
./build/qmlprofileranalyzer summary samples/minimal.qtd
./build/qmlprofileranalyzer types samples/minimal.qtd
```

実行デモ:

```bash
scripts/generate_demo_trace.sh
./build/qmlprofileranalyzer summary samples/generated/autotrace.qtd
```

補足:

- 実行デモは `qml` + `qmlprofiler --attach` を使う
- `qmlscene` は使わない
- `qml` ランタイム由来の `default.qml` 系イベントが追加で入る場合がある

実行形式デモ:

```bash
scripts/generate_executable_demo_trace.sh
./build/qmlprofileranalyzer summary samples/generated/localizedclockdemo.qtd
```

補足:

- `localizedclockdemo` を起動して `qmlprofiler --attach` する
- `localizedclockdemo` は約15秒で自動終了する
- `qml` デモより実運用に近い
- `localizedclockdemo` の記録は `docs/EXECUTABLE_DEMO_*` を参照

reader 品質:

- `tests/test_qtdreader.cpp` で正常系と異常系の自動テストを持つ
- 必須属性が欠けた `event` / `range` / `note` は読み飛ばす
- `.qzt` fixture を使った range / point event / note の読込テストを持つ
- `events` は raw event 専用
- `events-aggregated` では `.qzt` の `PixmapCache`, `SceneGraph`, `MemoryAllocation`, `DebugMessage` を比較しやすい集約行に整形し、Qt Creator 比較用ラベルも付ける

着手時の読む順番:

1. `docs/FORMAT.md`
2. `references/qtcreator/qmlprofiler/qmlprofilertracefile.cpp`
3. `references/qtdeclarative/qmlprofilerdata.cpp`
4. `references/qtdeclarative/qqmlprofilerclientdefinitions_p.h`
