# 02 ヘッダーと実装を分ける

## このレッスンで学ぶこと

`widget.h` に `Widget` class の宣言を書き、`widget.cpp` にコンストラクタの実装を書きます。

## 操作

```text
icpp[qtcling]> .e widget.h
# Widget クラスの宣言を書いて保存する
icpp[qtcling]> .e widget.cpp
# ラベルとボタンを作るコンストラクタを書いて保存する
icpp[qtcling]> .gen
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

ラベルとボタンを持つウィンドウが表示されます。

## ポイント

まだクリック処理は書きません。まずはヘッダーと実装を分ける基本形に慣れます。
