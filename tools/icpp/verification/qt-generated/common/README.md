# 共通

Qt 生成物検証で共通に使うスクリプトを置きます。

## run_case.sh

現在のディレクトリの `input.icpp` を icpp に流し、`output.txt` を作成します。

## clean_generated.sh

`moc_*.cpp`、`ui_*.h`、`qrc_*.cpp`、`*.qm`、`output.txt` を削除します。

## expected-format.md

各パターンの `expected.txt` の書き方です。
