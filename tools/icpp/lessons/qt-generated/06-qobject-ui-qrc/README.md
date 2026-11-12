# 06 Q_OBJECT + .ui + .qrc

## このレッスンで学ぶこと

`Q_OBJECT`、`.ui`、`.qrc` を同時に使う複合構成を学びます。

icpp で Qt 生成物を扱うときの総合練習です。

## 事前確認

このディレクトリに次のファイルがあります。

```text
widget.h
form.ui
resources.qrc
message.txt
widget.cpp
```

`widget.cpp` が評価対象です。それ以外は include または生成物の入力として使います。

## 操作

```text
icpp[qtcling]> .designer form.ui
icpp[qtcling]> .e widget.h
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .qrc resources.qrc
icpp[qtcling]> .gen
icpp[qtcling]> .defs
icpp[qtcling]> .files
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

- `moc_widget.cpp` が生成されます
- `ui_form.h` が生成されます
- `qrc_resources.cpp` が生成されます
- UI 上の label に resource の文字列が表示されます

## なぜそうするか

Qt プロジェクトでは、ヘッダー、UI、resource をまとめて扱うことがよくあります。icpp では `.gen` と `.add widget.cpp` の関係を理解すると、この構成も整理して扱えます。

## よくある失敗

- `.ui` や `.qrc` を評価対象だと思ってしまう
- `.i .` を忘れて生成物 include が見つからない
- 途中で生成物が古くなっているのに `.gen` しない

## 発展

`form.ui` のタイトルと `message.txt` の内容を同時に変更し、`.gen` 後に再確認してください。
