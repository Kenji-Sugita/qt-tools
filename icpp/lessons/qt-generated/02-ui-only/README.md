# 02 .ui Only

## このレッスンで学ぶこと

`.ui` から `ui_form.h` を生成し、C++ から使う基本形を学びます。

`.ui` は Qt Designer の XML ファイルです。icpp では `.gen` で `uic` を実行し、生成された `ui_form.h` を `widget.cpp` から include します。

## 事前確認

このディレクトリに次のファイルがあります。

```text
form.ui
widget.cpp
```

`widget.cpp` は `ui_form.h` を include しますが、最初はまだ生成されていません。

## 操作

```text
icpp[qtcling]> .designer form.ui
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .defs
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

- `.designer form.ui` では `form.ui` は自動登録されません
- `.e widget.cpp` では `widget.cpp` が自動登録されます
- `.gen` で `ui_form.h` が生成されます
- `.defs` に `function QWidget *go()` が表示されます
- `go()` で UI を組み立てた `QWidget` を生成できます

## なぜそうするか

`.ui` は C++ ではないため、icpp が直接評価する対象ではありません。`uic` の入力として扱い、生成された `ui_form.h` を C++ から使います。

## よくある失敗

- `.gen` 前に `widget.cpp` を評価すると、`ui_form.h` が見つからず失敗します
- `ui_form.h` を `.add` すると、ヘッダーを単体評価することになり扱いにくくなります
- `.designer form.ui` だけでは評価対象がないため、`.e widget.cpp` または `.add widget.cpp` が必要です

## 発展

`form.ui` の `windowTitle` を変更し、`.gen` と `static auto w = go();` を再実行してください。
