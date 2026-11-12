# 12 .inspect で実行中の Widget を調べる

## このレッスンで学ぶこと

最後に、icpp で作った GUI を `.inspect` で調べます。

`.inspect` を使うと、表示中の widget を選び、PropertyEditor でプロパティを見たり、Object Tree で部品の構造を見たり、Pick Object で画面上の部品を直接選んだりできます。

## 操作

```text
icpp[qtcling]> .add widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .r
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
icpp[qtcling]> .inspect
```

## 試すこと

- top-level widget selector で `12 Inspect Widget` を選びます
- PropertyEditor で `objectName`, `windowTitle`, `geometry` を探します
- `Show Object Tree` を押し、`titleLabel`, `previewLabel`, `sizeSlider`, `accentButton` を確認します
- `Pick Object` を押し、画面上のラベル、スライダー、ボタンをクリックします
- 赤いハイライトを消したいときは `Clear Pick` を押します
- スライダーやボタンを操作してから、もう一度 PropertyEditor の値を見ます

## 期待される結果

実行中の GUI の構造とプロパティを、icpp の中から確認できます。

この lesson は、icpp が単に C++ を実行するだけでなく、動かしている Qt widget をその場で観察できることを確認する締めくくりです。

## 片付け

```text
icpp[qtcling]> .widgets
icpp[qtcling]> .closeall
icpp[qtcling]> .q
```
