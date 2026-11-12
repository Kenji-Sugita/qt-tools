# 06 Designer UI を使う

## このレッスンで学ぶこと

`.ui` から `ui_form.h` を生成し、`Widget` クラスに組み込んで動かします。

## 操作

```text
icpp[qtcling]> .designer form.ui
# Qt Designer で form.ui を保存する
icpp[qtcling]> .gen
icpp[qtcling]> .e widget.h
# ui_form.h を使う Widget クラスを書いて保存する
icpp[qtcling]> .e widget.cpp
# setupUi(this) で form.ui の部品を使う実装を書いて保存する
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
icpp[qtcling]> .clean
icpp[qtcling]> .generated
```

## 期待される結果

UI ファイルで配置したボタンとスライダーが表示されます。ボタンを押すとラベルの表示が変わり、スライダーを動かすと数値が変わります。

## ポイント

`.designer` は Qt Designer を開くとすぐ icpp に戻ります。UI を変更したら、Qt Designer 側で保存してから `.gen` を実行してください。

`form.ui` は `uic` の入力です。`ui_form.h` がまだない状態で `widget.cpp` を編集するとエラーになります。
