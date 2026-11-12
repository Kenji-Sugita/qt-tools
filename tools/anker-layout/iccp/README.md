# 操作例

`anker-layout/` を作業ディレクトリーとして、Qt Cling を使う icpp を起動します。
ディレクトリー名 `iccp/` は正本の配布構成を保持しています。

```bash
icpp --engine qtcling
```

対角にボタンを配置する例です。

```text
icpp[qtcling]> .i include
icpp[qtcling]> .add src/AnkerFormAttachment.cpp
icpp[qtcling]> .add src/AnkerFormLayout.cpp
icpp[qtcling]> .add iccp/diagonal.cpp
icpp[qtcling]> .files
icpp[qtcling]> .r
icpp[qtcling]> .x static auto w = go();
```

`.files` で登録した3ファイルを確認し、`.r` で評価してから `go()` を呼びます。
基本フォームを試す場合は、新しいセッションで `iccp/diagonal.cpp` の代わりに
`iccp/anker_icpp.cpp` を登録してください。両ファイルとも `go()` を定義するため、同時には登録しません。

[English](README.en.md) / [Anker Layout](../README.md)
