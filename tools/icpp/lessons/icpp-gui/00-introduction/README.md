# 00 はじめに

## このレッスンで学ぶこと

icpp の基本操作を確認します。この教材では、C++ を外部エディタで書き、icpp から生成・評価し、`w->show()` で画面を出します。

## よく使うコマンド

```text
.e widget.cpp     ファイルをエディタで開き、閉じた後に評価
.gen              moc/uic/rcc を生成してから再評価
.defs             定義された関数や class を確認
.files            登録ファイルを確認
.status           現在状態を確認
.errors           直近のエラーを確認
```

## 基本の流れ

```text
icpp[qtcling]> .e widget.cpp
# Widget クラスと go() を書いて保存する
icpp[qtcling]> .gen
icpp[qtcling]> .defs
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## ポイント

`go()` は、この教材で使う共通の入り口です。実際の処理は `Widget` class に書き、`go()` は基本的に `return new Widget;` だけにします。

`deleteLater()` はこの教材では使いません。最初は、作った widget を画面に表示して挙動を見ることを優先します。
