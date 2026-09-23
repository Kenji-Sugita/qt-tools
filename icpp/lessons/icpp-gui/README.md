# icpp はじめてレッスン: GUI を動かしながら覚える Qt REPL

この教材は、初めて icpp を使う人が、実際に小さな GUI を動かしながら操作を覚えるためのものです。

`verification/` は自動検証用ですが、この `lessons/icpp-gui/` は手で動かす練習用です。各 lesson のディレクトリへ移動して、README の手順を icpp で実行します。

```sh
cd lessons/icpp-gui/01-first-window
../../build/icpp
```

## 目次

| Lesson | 内容 |
|---|---|
| `00-introduction` | 進め方と基本コマンド |
| `01-first-window` | ヘッダーのみで小さな Widget を作る |
| `02-button-label` | ヘッダーと実装を分ける |
| `03-button-counter` | lambda connect でボタンとラベルを動かす |
| `04-slider-state` | スライダーを class の状態に接続する |
| `05-qobject-slot` | `Q_OBJECT` と slot を使う |
| `06-designer-ui` | Designer UI を class に組み込む |
| `07-resource-message` | resource を class で使う |
| `08-resource-update` | resource を変更して再生成する |
| `09-autogen` | `.autogen on` で手順を短くする |
| `10-translation` | 翻訳を class に組み込む |
| `11-fix-errors` | 失敗を読んで直す |
| `12-inspect-widget` | `.inspect` で実行中の Widget を調べる |
| `appendix-a-commands` | よく使うコマンド |
| `appendix-b-generated-files` | 生成物の考え方 |

## 基本の形

ほとんどの lesson は、`widget.h` に class 宣言、`widget.cpp` に実装を書きます。`go()` は REPL から class を起動するための薄い入口です。

```text
icpp[qtcling]> .e widget.h
icpp[qtcling]> .e widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

通常入力は既定では編集バッファに残りません。`static auto w = go();` や `w->show();` は、そのまま入力して試します。

## 注意

この教材は GUI を表示するため、画面のある環境で実行してください。画面のない環境で `show()` すると `Cannot create window: no screens available` になることがあります。
