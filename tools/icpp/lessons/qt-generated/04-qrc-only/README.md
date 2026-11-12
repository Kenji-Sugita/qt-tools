# 04 .qrc Only

## このレッスンで学ぶこと

`.qrc` から `qrc_resources.cpp` を生成し、resource 内のテキストを読む流れを学びます。

画像やテキストなどを Qt resource として埋め込む場合、`rcc` による生成が必要です。

## 事前確認

このディレクトリに次のファイルがあります。

```text
resources.qrc
message.txt
widget.cpp
```

`resources.qrc` は `message.txt` を resource に含めます。

## 操作

```text
icpp[qtcling]> .qrc resources.qrc
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .defs
icpp[qtcling]> .files
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

- `.qrc resources.qrc` では `resources.qrc` は自動登録されません
- `.e widget.cpp` では `widget.cpp` が自動登録されます
- `.gen` で `qrc_resources.cpp` が生成されます
- `go()` で `message.txt` の内容を表示する widget が作られます

## なぜそうするか

`.qrc` は C++ ではなく、`rcc` の入力です。生成された `qrc_resources.cpp` を `widget.cpp` から include することで resource を使えるようにします。

## よくある失敗

- `.qrc` を編集しただけでは生成物は更新されません
- `qrc_resources.cpp` を直接 `.add` すると、生成コードの構造でつまずく場合があります
- resource path を間違えると `open failed` になります

## 発展

`message.txt` の内容を変更し、`.gen` を再実行してから、もう一度 `go()` で表示を確認してください。
