# 11 壊れた UI の失敗系

壊れた `.ui` で `.gen` が失敗し、再評価に進まないことを確認します。

このケースは失敗系なので、`output.txt` に `error` や `External command exited` が出ることを期待します。

## 自動確認

```sh
../common/run_case.sh
```
