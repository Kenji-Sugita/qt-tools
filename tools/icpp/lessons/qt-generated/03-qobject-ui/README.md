# 03 Q_OBJECT + .ui

## このレッスンで学ぶこと

`Q_OBJECT` と `.ui` を同時に使う構成を学びます。

このケースでは、`.gen` によって `moc_widget.cpp` と `ui_form.h` の両方が必要になります。

## 事前確認

このディレクトリに次のファイルがあります。

```text
widget.h
form.ui
widget.cpp
```

`widget.cpp` は `widget.h` と `ui_form.h` を include し、末尾で `moc_widget.cpp` を include します。

## 操作

```text
icpp[qtcling]> .e widget.h
icpp[qtcling]> .designer form.ui
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .defs
icpp[qtcling]> .files
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

- `.gen` で `moc_widget.cpp` が生成されます
- `.gen` で `ui_form.h` が生成されます
- `.defs` に `go()` が表示されます
- `go()` で `Widget` を生成できます

## なぜそうするか

`Q_OBJECT` は `moc`、`.ui` は `uic` の入力です。icpp では `.gen` がそれらをまとめて更新します。

評価対象は `widget.cpp` です。ヘッダーや生成物を直接評価するのではなく、通常の include 関係に寄せます。

## よくある失敗

- `widget.h` を `.add` してしまうと、ヘッダー単体評価になり class 定義の扱いでつまずきやすくなります
- `ui_form.h` 生成前に評価すると include で失敗します
- slot を追加したあと `.gen` を忘れると、古い moc 情報で動いているように見えることがあります

## 発展

`widget.h` に簡単な public method を追加し、`.gen` と `.r` を試してください。
