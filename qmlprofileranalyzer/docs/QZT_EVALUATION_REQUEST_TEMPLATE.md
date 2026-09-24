# QZT Evaluation Request Template

## 目的

この文書は、`.qzt` と対応ソースコードから
`..._QZT_EVALUATION.md` を作成させるための最小テンプレートである。

## 最小テンプレート

```text
QZT_EVALUATION_AGENT_INSTRUCTIONS.md に従って、
<trace.qzt> と対応ソースコードから
docs/<NAME>_QZT_EVALUATION.md を作成してください。
```

## 推奨テンプレート

```text
docs/QZT_EVALUATION_AGENT_INSTRUCTIONS.md に従って、
次の入力から評価文書を作成してください。

- .qzt: /path/to/trace.qzt
- ソースコード:
  - /path/to/main.qml
  - /path/to/helper.js
  - /path/to/Component.qml
- 出力先: docs/TRACE_QZT_EVALUATION.md

要求:
- summary / types / events を根拠にすること
- events-aggregated も確認すること
- 行番号とソースコードを対応付けること
- 重い処理と軽い処理を分けること
- 必要なら Qt Creator との比較を入れること
- 推測は推測と分かるように書くこと
```

## 使い方

毎回差し替える項目は次の 3 つだけ。

1. `.qzt` のパス
2. 対応ソースコードのパス
3. 出力ファイル名

## samegame での例

```text
docs/QZT_EVALUATION_AGENT_INSTRUCTIONS.md に従って、
次の入力から評価文書を作成してください。

- .qzt: /Users/sugita/src/qt/tools/qmlprofileranalyzer/tests/data/samegame.qzt
- ソースコード:
  - /usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.qml
  - /usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.js
  - /usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/Block.qml
  - /usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/Button.qml
- 出力先: docs/SAMEGAME_QZT_EVALUATION.md

要求:
- summary / types / events を根拠にすること
- events-aggregated も確認すること
- 行番号とソースコードを対応付けること
- 重い処理と軽い処理を分けること
- 必要なら Qt Creator との比較を入れること
- 推測は推測と分かるように書くこと
```

## 関連文書

- [QZT_EVALUATION_AGENT_INSTRUCTIONS.md](/Users/sugita/src/qt/tools/qmlprofileranalyzer/docs/QZT_EVALUATION_AGENT_INSTRUCTIONS.md)
- [QZT_EVALUATION_WORK_INSTRUCTIONS.md](/Users/sugita/src/qt/tools/qmlprofileranalyzer/docs/QZT_EVALUATION_WORK_INSTRUCTIONS.md)
- [QZT Evaluation Procedure](/Users/sugita/src/qt/tools/qmlprofileranalyzer/docs/SAMEGAME_QZT_PROCEDURE.md)
