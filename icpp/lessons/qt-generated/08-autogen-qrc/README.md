# 08 autogen + .qrc

## このレッスンで学ぶこと

`.autogen on` にすると、`.e` や `.r` の再評価前に必要な Qt 生成物を自動更新できることを学びます。

通常は `.gen` を明示しますが、繰り返し編集する場合は autogen が便利です。

## 事前確認

このディレクトリに次のファイルがあります。

```text
resources.qrc
message.txt
widget.cpp
```

## 操作

```text
icpp[qtcling]> .autogen on
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .defs
icpp[qtcling]> .files
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

- `Autogen: on` が表示されます
- `.e widget.cpp` の再評価前に `Autogen: run_all` が表示されます
- `qrc_resources.cpp` が生成されます
- `go()` で resource の内容を表示する widget が作られます

## なぜそうするか

`.autogen on` は、Qt 生成物が必要そうなときだけ `.e` や `.r` の再評価前に `run_all` を実行します。`.gen` を忘れにくくするための補助機能です。

## よくある失敗

- autogen が off のままだと `.r` だけでは生成物が更新されません
- 未保存の編集バッファがある場合、autogen の前に保存が必要になることがあります
- 意図せず生成が走るのを避けたい場合は `.autogen off` に戻します

## 発展

`.status` を実行して、autogen の状態と登録ファイルを確認してください。
