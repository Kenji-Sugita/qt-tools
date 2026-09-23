# 01 ヘッダーのみで小さな Widget を作る

## このレッスンで学ぶこと

`widget.h` だけで小さな `Widget` クラスを定義し、`widget.cpp` の `go()` から表示します。

## 操作

```text
icpp[qtcling]> .e widget.h
# Widget クラスをヘッダー内に書いて保存する
icpp[qtcling]> .e widget.cpp
# go() を書いて保存する
icpp[qtcling]> .gen
icpp[qtcling]> .defs
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

タイトル付きの小さなウィンドウが表示されます。

## ポイント

最初はコンストラクタをヘッダー内に inline で書きます。小さいクラスならこの形でも試しやすく、icpp での最初の練習に向いています。

## よくある失敗

- `static auto w = go();` の前に `.gen` を忘れる
- `go()` の戻り値を保存せずに `go()->show();` だけ実行し、後で操作しづらくなる
