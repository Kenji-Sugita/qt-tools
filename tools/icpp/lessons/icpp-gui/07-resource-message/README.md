# 07 リソースを使う

## このレッスンで学ぶこと

`.qrc` に入れた `message.txt` を、`Widget` クラスのラベルに表示します。

## 操作

```text
icpp[qtcling]> .qrc resources.qrc
# resources.qrc を確認して保存する
icpp[qtcling]> .gen
icpp[qtcling]> .e widget.h
# リソースを読む Widget クラスを書いて保存する
icpp[qtcling]> .e widget.cpp
# qrc_resources.cpp をインクルードし、message.txt を表示する実装を書いて保存する
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
icpp[qtcling]> .clean
icpp[qtcling]> .generated
```

## 期待される結果

リソースから読んだメッセージがウィンドウに表示されます。

## ポイント

`qrc_resources.cpp` は直接 `.add` せず、`widget.cpp` からインクルードします。

`resources.qrc` は `rcc` の入力です。`qrc_resources.cpp` がまだない状態で `widget.cpp` を編集するとエラーになるので、先に `.gen` をします。

`.qrc` はエディタを開く操作です。リソースファイルを確認したら保存して閉じ、icpp に戻って `.gen` を実行してください。
