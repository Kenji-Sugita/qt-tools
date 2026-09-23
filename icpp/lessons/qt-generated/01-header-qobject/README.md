# 01 Header + Q_OBJECT

## このレッスンで学ぶこと

`Q_OBJECT` を持つ class をヘッダーに置き、icpp から `moc` 生成物を使う基本形を学びます。

Qt の `signals` / `slots` / `Q_PROPERTY` などを使う class では `moc` が必要です。icpp では `.gen` を使って `moc_widget.cpp` を生成し、評価し直します。

## 事前確認

このディレクトリに次のファイルがあります。

```text
widget.h
widget.cpp
```

`widget.cpp` の末尾では、生成される `moc_widget.cpp` を include します。

```cpp
#include "moc_widget.cpp"
```

## 操作

```text
icpp[qtcling]> .e widget.h
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .defs
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

- `.e widget.h` では `widget.h` は自動登録されません
- `.e widget.cpp` では `widget.cpp` が自動登録されます
- `.gen` で `moc_widget.cpp` が生成されます
- `.defs` に `function Widget *go()` が表示されます
- `static auto w = go();` で `Widget` を生成できます

## なぜそうするか

`widget.h` は C++ として評価する対象ではなく、`widget.cpp` から include されるヘッダーです。

`moc_widget.cpp` も直接 `.add` せず、`widget.cpp` から include します。生成物を個別に登録するより、通常の C++ ファイルの include 関係に寄せる方が安定します。

## よくある失敗

- `.i .` を忘れると、`#include "widget.h"` や `#include "moc_widget.cpp"` が見つからないことがあります
- `.gen` を忘れると、`moc_widget.cpp` が存在せず include で失敗します
- `moc_widget.cpp` を直接 `.add` すると、生成コードの評価順でつまずくことがあります

## 発展

`widget.h` に slot や `Q_PROPERTY` を追加し、`.gen` を再実行してから `.defs` と `static auto w = go();` を試してください。
