# 05 Q_OBJECT とスロットを使う

## このレッスンで学ぶこと

`Q_OBJECT` を持つクラスをヘッダーに分け、スロットを使います。`.gen` で `moc_widget.cpp` を生成します。

## 操作

```text
icpp[qtcling]> .e widget.h
# Q_OBJECT とスロットを持つ Widget クラスを書いて保存する
icpp[qtcling]> .e widget.cpp
# clicked をスロットへ接続し、moc_widget.cpp をインクルードして保存する
icpp[qtcling]> .generated
icpp[qtcling]> .! ls -l moc_widget.cpp
icpp[qtcling]> .! cat moc_widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
icpp[qtcling]> .clean
icpp[qtcling]> .generated
```

## 期待される結果

ボタンを押すたびに、スロットが呼ばれてカウントが増えます。

## ポイント

`moc_widget.cpp` がまだない場合、icpp は空のプレースホルダーでインクルードエラーを避けることがあります。ただし本物ではないので、必ず `.gen` で生成します。
