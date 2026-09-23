# 04 スライダーを class の状態に接続する

## このレッスンで学ぶこと

`Widget` クラスに `QSlider` と `QLabel` を持たせ、プライベートメソッドで表示を更新します。

## 操作

```text
icpp[qtcling]> .e widget.h
# ラベル、スライダー、更新用メソッドを持つ Widget クラスを書いて保存する
icpp[qtcling]> .e widget.cpp
# スライダーの値をラベルへ反映する実装を書いて保存する
icpp[qtcling]> .gen
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

スライダーを動かすと、ラベルの数値が変わります。
