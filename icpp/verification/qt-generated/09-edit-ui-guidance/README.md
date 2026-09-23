# 09 UI 編集時の案内

`.e form.ui` の直後に評価対象の C++ ファイルがない場合、汎用的な `No files or edit buffer to run.` ではなく、`.e <file.cpp>` または `.add <file.cpp>` を促す案内が出ることを確認します。

## 自動確認

```sh
../common/run_case.sh
```
