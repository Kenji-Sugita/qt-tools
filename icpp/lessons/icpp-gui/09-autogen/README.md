# 09 autogen で楽にする

## このレッスンで学ぶこと

`.autogen on` を使い、`.e` 後の再評価前に自動で `run_all` が走ることを確認します。

## 操作

```text
icpp[qtcling]> .autogen
icpp[qtcling]> .autogen on
icpp[qtcling]> .status
icpp[qtcling]> .e widget.h
# リソースを読む Widget クラスを書いて保存する
icpp[qtcling]> .e widget.cpp
# autogen で生成される qrc_resources.cpp を使う実装を書いて保存する
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
icpp[qtcling]> .clean
icpp[qtcling]> .generated
```

## 期待される結果

`.e widget.cpp` のあとに `Autogen: run_all` が表示され、リソースを使うウィジェットが表示されます。
