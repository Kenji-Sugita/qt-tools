# 11 失敗を直す

## このレッスンで学ぶこと

`.ui` の objectName と C++ 側の参照名がずれているとき、icpp の診断と `.uiinfo` で原因を確認します。

## 操作

```text
icpp[qtcling]> .designer form.ui
# form.ui を確認して保存する
icpp[qtcling]> .gen
icpp[qtcling]> .e widget.h
# ui_form.h を private 継承する Widget クラスを書いて保存する
icpp[qtcling]> .e widget.cpp
# 存在しない messageLabel を参照する実装を書いて保存し、診断を確認する
icpp[qtcling]> .uiinfo form.ui
icpp[qtcling]> .e widget.cpp
# messageLabel を titleLabel に変更する
# 保存して icpp に戻る
icpp[qtcling]> .gen
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
icpp[qtcling]> .clean
```

## 期待される結果

`widget.cpp` の評価で、`messageLabel` が `Widget` または `Ui::Form` に存在しないというエラーが表示されます。

`.uiinfo form.ui` では、実際のラベルの objectName が `titleLabel` であることを確認できます。

## 発展

`widget.cpp` は、存在しない objectName を参照しています。

```cpp
messageLabel->setText("Fixed");
```

`form.ui` の中ではラベルの objectName は `titleLabel` です。C++ 側を次のように直してください。

```cpp
titleLabel->setText("Fixed");
```

修正後、もう一度 `.gen` を実行してください。`go()` でウィジェットを作れることを確認できます。
