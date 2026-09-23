# 10 .qrc 再生成

## このレッスンで学ぶこと

resource の元ファイルを変更した後、`.gen` で `qrc_resources.cpp` を再生成する必要があることを学びます。

## 事前確認

このディレクトリに次のファイルがあります。

```text
resources.qrc
message.txt
widget.cpp
```

`message.txt` は操作の中で書き換えます。

## 操作

```text
icpp[qtcling]> .add widget.cpp
icpp[qtcling]> .! printf 'first resource\n' > message.txt
icpp[qtcling]> .gen
icpp[qtcling]> static auto first = go();
icpp[qtcling]> first->show();
icpp[qtcling]> first->raise();
icpp[qtcling]> .! printf 'second resource\n' > message.txt
icpp[qtcling]> .gen
icpp[qtcling]> static auto second = go();
icpp[qtcling]> second->show();
icpp[qtcling]> second->raise();
```

## 期待される結果

- 1 回目の `go()` は `first resource` を表示する widget を作ります
- 2 回目の `go()` は `second resource` を表示する widget を作ります

## なぜそうするか

`message.txt` を変えただけでは、すでに生成された `qrc_resources.cpp` は変わりません。resource を読み直すには `.gen` で `rcc` を再実行します。

## よくある失敗

- `message.txt` を変更しただけで `.gen` を忘れる
- `qrc_resources.cpp` が古いままなので、前の値が表示される
- shell command の `>` は `.!` 経由で実行する必要があります

## 発展

`resources.qrc` に別のファイルを追加し、`.gen` 後に C++ 側から読めるか試してください。
