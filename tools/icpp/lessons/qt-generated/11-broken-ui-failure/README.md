# 11 objectName 不一致の失敗系

## このレッスンで学ぶこと

`.ui` の objectName と C++ 側の参照名がずれているとき、interpreter の診断と `.uiinfo` で原因を確認する流れを学びます。

成功するケースだけでなく、失敗時にどこを見るかを知っておくと、icpp での試行錯誤がしやすくなります。

## 事前確認

このディレクトリに次のファイルがあります。

```text
form.ui
widget.cpp
```

`form.ui` は正しい XML です。ただし、`widget.cpp` は存在しない objectName を参照するように意図的に間違えてあります。

## 操作

```text
icpp[qtcling]> .add widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .uiinfo form.ui
```

## 期待される結果

- `widget.cpp` の評価で `messageLabel` が `Ui::Form` に存在しないというエラーが表示されます
- interpreter の診断で、どの名前が存在しないか確認できます
- `.uiinfo form.ui` で、実際の label の objectName が `titleLabel` であることを確認できます

## なぜそうするか

Designer で widget の objectName を変えたあと、C++ 側の `ui->...` や `ui....` 参照を直し忘れることがあります。この場合、`.ui` の生成は成功しますが、生成された `ui_form.h` に存在しない名前を C++ から参照するため評価に失敗します。

## よくある失敗

- `widget.cpp` だけを見て、`.ui` 側の objectName を確認しない
- interpreter の診断を見ずに `.gen` だけ繰り返す
- `ui_form.h` を直接直そうとする

## 発展

`widget.cpp` は、存在しない objectName を参照しています。

```cpp
ui.messageLabel->setText("Fixed");
```

`form.ui` の中では label の objectName は `titleLabel` です。C++ 側を `ui.titleLabel->setText("Fixed");` に直してから、もう一度 `.gen` を実行してください。
