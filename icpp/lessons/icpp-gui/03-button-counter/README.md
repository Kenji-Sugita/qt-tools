# 03 lambda connect でボタンとラベルを動かす

## このレッスンで学ぶこと

`Widget` class に `QLabel *label` と `int count` を持たせ、lambda connect でボタン操作を処理します。

## 操作

```text
icpp[qtcling]> .e widget.h
# ラベルと count を member に持つ Widget クラスを書いて保存する
icpp[qtcling]> .e widget.cpp
# ボタンの clicked を lambda に接続する実装を書いて保存する
icpp[qtcling]> .gen
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

ボタンを押すたびに、ラベルのカウントが増えます。
