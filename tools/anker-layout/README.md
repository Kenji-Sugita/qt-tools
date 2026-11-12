# Anker Layout

[English](README.en.md)

Anker Layout は、Qt Widgets で Motif の `XmForm` に近いフォーム型レイアウトを使うための `QLayout` 実装です。
ウィジェットの上下左右の辺を、フォームの端、他のウィジェット、割合位置、または自身の初期位置に結び付けて配置します。
Motif のライブラリーは必要ありません。

## できること

- フォームの端への固定と、両側を固定したウィジェットの伸縮
- 他のウィジェットとの間隔指定、上下左右の辺揃え
- フォームの幅・高さに対する割合位置への配置
- 初期位置を基準にした配置と、フォームのリサイズに応じた比率移動
- `setConstraints()` による制約の変更と、`constraintsFor()` による取得

`Anker::FormLayout` の `addWidget()` に `FormConstraints` を渡します。例えば、ラベルの右に入力欄を置き、入力欄をフォームの右端まで伸ばす指定は次のようになります。

```cpp
layout->addWidget(editor, {
    .left = Anker::attachWidget(label, 8),
    .right = Anker::attachForm(),
    .top = Anker::attachOppositeWidget(label),
});
```

ライブラリー本体は `include/` のヘッダー3件と `src/` の実装2件です。
使用するコードでは `AnkerFormLayout.h` をインクルードします。

## ビルドとテスト

- C++20 対応コンパイラー
- CMake 3.16 以上
- Qt 6 の Widgets と Test モジュール（全体ビルド時）

このディレクトリーで実行します。

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Qt が見つからない場合は、構成時に `-DCMAKE_PREFIX_PATH=/path/to/Qt` で使用する Qt のインストール先を指定してください。
画面を使わずにテストする場合は、`QT_QPA_PLATFORM=offscreen` を設定して CTest を実行できます。

全体ビルドではライブラリー、6つのサンプル、テストを生成します。サンプルは単体でもビルドでき、その場合は Qt Widgets を使用します。

```bash
cmake -S examples/basic_form -B build-basic_form
cmake --build build-basic_form
```

## サンプル

| ディレクトリー | 実行ターゲット | 内容 |
|---|---|---|
| `examples/basic_form` | `anker_basic_form` | ラベル、入力欄、ボタンの基本フォーム |
| `examples/relative_widgets` | `anker_relative_widgets` | 他のウィジェットを基準にした配置 |
| `examples/position_anchors` | `anker_position_anchors` | フォームを割合位置で分割 |
| `examples/opposite_edges` | `anker_opposite_edges` | 複数ウィジェットの辺揃え |
| `examples/resizable_dialog` | `anker_resizable_dialog` | リサイズに追従するダイアログ |
| `examples/full_attachments` | `anker_full_attachments` | 各種 attachment と spacing の組み合わせ |

全体ビルドのサンプルは `build/examples/<サンプル名>/` に生成されます。macOS では `.app` バンドルになります。例えば基本フォームは次のように起動します。

```bash
open build/examples/basic_form/anker_basic_form.app
```

Qt Cling / icpp で試すコードも `iccp/` に収録しています。操作方法は [icppでの操作例](iccp/README.md) を参照してください。対話実行には別途 Qt Cling と icpp が必要です。

## 利用ガイドと配布物

- [利用ガイド（Markdown）](USER_GUIDE.md)
- [図付き利用ガイド（PDF）](USER_GUIDE.pdf)
- [Mermaid版利用ガイド（Markdown）](USER_GUIDE_MERMAID.md) / [PDF](USER_GUIDE_MERMAID.pdf)
- [正本の配布ZIP](anker-layout.zip)

通常版の Markdown は genpdf 用の `!dot` 記法を含みます。図はPDFで確認できます。

このディレクトリーには、Sites の配布ZIPから取り出したソース、サンプル、テスト、利用ガイドを収録しています。ZIPとPDFは正本の配布物をそのまま保持しています。公開用README、英訳、`iccp/` の操作案内は、このリポジトリー向けに追加・調整しています。

循環する attachment は避けてください。`attachSelf()` の初期位置は `addWidget()` の前に設定します。`attachOppositeForm()` は offset の符号によってフォーム外へ配置されるため、詳細は利用ガイドを参照してください。
