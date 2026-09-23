# 08 リソースを変更して反映する

## このレッスンで学ぶこと

リソースの元ファイルを変更したら、`.gen` で `qrc_resources.cpp` を再生成する必要があることを学びます。

## 操作

```text
icpp[qtcling]> .e widget.h
# リソースを読む Widget クラスを書いて保存する
icpp[qtcling]> .e widget.cpp
# qrc_resources.cpp をインクルードし、message.txt を表示する実装を書いて保存する
icpp[qtcling]> static auto first = go();
icpp[qtcling]> first->show();
icpp[qtcling]> .e message.txt
# message.txt を Second message に変更して保存する
icpp[qtcling]> .gen
icpp[qtcling]> static auto second = go();
icpp[qtcling]> second->show();
icpp[qtcling]> second->raise();
icpp[qtcling]> .clean
icpp[qtcling]> .generated
```

## 期待される結果

最初のウィンドウと、変更後のウィンドウで表示されるメッセージが変わります。

## ポイント

`message.txt` はリソースの入力です。ファイルを変更しただけでは、既に生成済みの `qrc_resources.cpp` は変わりません。保存後に `.gen` を実行し、リソース生成物を更新します。
