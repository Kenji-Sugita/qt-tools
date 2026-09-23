# 07 翻訳

`.ts` を `lrelease` で `.qm` にし、`.qrc` 経由で `QTranslator` が読めることを確認します。

現時点では `.gen` は `lrelease` を実行しません。`input.icpp` では `.! lrelease app_ja.ts` を明示します。

## 自動確認

```sh
../common/run_case.sh
```
