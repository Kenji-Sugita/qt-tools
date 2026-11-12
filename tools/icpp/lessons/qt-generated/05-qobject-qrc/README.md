# 05 Q_OBJECT + .qrc

## このレッスンで学ぶこと

`Q_OBJECT` と `.qrc` を同時に使う構成を学びます。

このケースでは、`moc_widget.cpp` と `qrc_resources.cpp` の 2 種類の生成物が必要です。

## 事前確認

このディレクトリに次のファイルがあります。

```text
widget.h
resources.qrc
message.txt
widget.cpp
```

`widget.cpp` は `widget.h`、`qrc_resources.cpp`、`moc_widget.cpp` を include します。

## 操作

```text
icpp[qtcling]> .e widget.h
icpp[qtcling]> .qrc resources.qrc
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .defs
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

- `.gen` で `moc_widget.cpp` が生成されます
- `.gen` で `qrc_resources.cpp` が生成されます
- `go()` で resource の文字列を表示する `Widget` を生成できます

## なぜそうするか

`Q_OBJECT` は `moc`、`.qrc` は `rcc` の入力です。どちらも `.gen` でまとめて更新します。

## よくある失敗

- `Q_OBJECT` を変えたのに `.gen` しない
- resource を変えたのに `.gen` しない
- 生成物を直接 `.add` して評価順が崩れる

## 発展

`message.txt` と `widget.h` の両方を変更し、`.gen` を 1 回実行してから確認してください。
