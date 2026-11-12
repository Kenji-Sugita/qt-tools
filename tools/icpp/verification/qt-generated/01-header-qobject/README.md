# 01 ヘッダー + Q_OBJECT

ヘッダー分離した `Q_OBJECT` クラスを icpp で扱えることを確認します。

## 自動確認

```sh
../common/run_case.sh
```

## 手動確認

```text
icpp[qtcling]> .i .
icpp[qtcling]> .add widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .x auto w = go();
icpp[qtcling]> .x w->show();
icpp[qtcling]> .widgets
```
