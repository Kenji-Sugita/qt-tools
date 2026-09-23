---
genpdf:
  Format: book
  Title: Property Editor 利用ガイド
  Subtitle: 1.0 版
  Author: (株) SRA
---

# 1. 概要

`property_editor` は、Qt Designer の内部実装に依存せず、`QObject` / `QMetaObject` /
`QMetaProperty` を使ってオブジェクトのプロパティーを表示・編集するためのツールです。

このガイドでは、単にビルド手順だけを説明するのではなく、**Cling から `PropertyEditor`**
を使って Qt オブジェクトを観察・編集するという観点で使い方を整理します。

前提として重要なのは次の点です。

- 対象は `QObject` ベースのオブジェクトです
- 表示される内容は `QMetaObject` と dynamic property に基づきます
- 編集できるかどうかは、プロパティーの型や writable 属性に依存します

`property_editor` は、Qt Designer の完全な代替ではなく、**Cling 上で Qt の状態を確認しながら
プロパティーを手早く試すための軽量なプロパティーエディター**として使うのが適しています。

# 2. 何ができるか

`PropertyEditor` を対象オブジェクトに接続すると、主に次のことができます。

- `QMetaProperty` に基づくプロパティー一覧の表示
- dynamic property の表示
- 型に応じたエディターによる値編集
- notify signal や部品固有 signal による表示更新
- 必要に応じた timer ベースの再読込

対応している代表的な型は次のとおりです。

- `bool`
- `int`
- `double`
- `QString`
- `QStringList`
- enum
- flags (`QFlags`)
- `QPoint`
- `QSize`
- `QRect`
- `QColor`
- `QByteArray`
- dynamic property

# 3. 基本方針

このツールは、すべての Qt プロパティーを一律に同じ方法で扱うのではなく、次の方針で更新と編集を行います。

1. `QMetaProperty::hasNotifySignal()` が使える場合は notify signal を優先する
2. `QSlider` や `QComboBox` など、よく使うウィジェットは部品固有 signal にも接続する
3. notify signal が無い、または拾い切れない場合は timer で保険を掛ける

このため、単に値を表示するだけでなく、Cling から部品を触ったときの変化追従も意識した作りになっています。

# 4. ビルド

通常のビルドは次のとおりです。

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos

cmake --build build
```

生成される主なファイルは次のとおりです。

- `build/property_editor.dylib`
- `build/property_editor_demo`

release version はルートの `VERSION` file で管理します。
`CMakeLists.txt` はこの値を読んで `project(property_editor VERSION ...)` を構成します。

# 5. インストール

`property_editor` は、`icpp` の `.inspect` から使うことを前提に、`icpp` と同じ
install prefix へ入れる運用を想定しています。

`cmake` configure 時に `CMAKE_INSTALL_PREFIX` を明示しなければ、`PATH` 上の
`icpp` を探し、その install prefix を既定値として使います。たとえば `icpp` が
`/usr/local/bin/icpp` にある場合は、`property_editor` も `/usr/local` へ
install されます。

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos

cmake --build build
cmake --install build
```

このとき主な install 先は次のとおりです。

- `bin/property_editor_demo`
- `lib/property_editor.dylib` (`macOS`) / `lib/property_editor.so` (`Linux`)
- `include/objectpicker.h`
- `include/objecttreedialog.h`
- `include/objecttreewidget.h`
- `include/propertyeditor.h`
- `include/propertyeditorglobal.h`
- `VERSION`

この配置により、`icpp/bin/icpp` から見て `../lib/property_editor.*` と
`../include/*.h` が揃うため、`.inspect` で自動検出できます。

`icpp` が無い環境や、別の場所に install したい場合は `CMAKE_INSTALL_PREFIX` を
明示してください。

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DCMAKE_INSTALL_PREFIX=$HOME/.local

cmake --build build
cmake --install build
```

`PATH` 上の `icpp` ではなく、特定の `icpp` install 先を基準にしたい場合は
`ICPP_PREFIX` で上書きできます。

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DICPP_PREFIX=/custom/icpp/prefix
```

# 6. source code release zip

配布 zip は source code release として扱います。ビルド済みバイナリーや Qt 本体は含めません。

- release version はルートの `VERSION` file で管理します
- 配布 zip は `release/` に置きます
- zip 名は `propertyeditor-<version>-source.zip` とします
- zip top directory は `propertyeditor-<version>/` とします

作成コマンド:

```bash
./scripts/create_release_zip.sh
```

現在の zip には、`CMakeLists.txt`、`VERSION`、`USER_GUIDE.md`、`src/`、`examples/`、
`tests/` を含めます。`build/`、`install/`、`.DS_Store`、一時ファイルは含めません。

# 7. テスト

テスト込みでビルドする場合は次のとおりです。

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DBUILD_TESTING=ON

cmake --build build
ctest --test-dir build --output-on-failure
```

自動テストでは、主に次を確認します。

- dynamic property を含むプロパティーツリー構築
- bool プロパティーのチェックボックス編集
- int プロパティーの `QSpinBox` 編集
- enum / flags の編集
- `QWidget::cursor` のコンボボックス編集
- 外部変更時の値追従
- `QPoint` / `QSize` / `QRect` / `QStringList` / `QByteArray` の文字列変換

# 8. まず試すべき最小例

最小の使い方は、調べたい `QObject` を用意し、それを `PropertyEditor` に渡すだけです。

```cpp
#pragma cling load("/Users/sugita/src/qt/tools/propertyeditor/build/property_editor.dylib")
#include "propertyeditor.h"
#include <QSlider>
#include <QVBoxLayout>

auto *previewWindow = new QWidget;
auto *previewLayout = new QVBoxLayout(previewWindow);

auto *slider = new QSlider(Qt::Horizontal);
slider->setRange(0, 500);
previewLayout->addWidget(slider);
previewWindow->show();

auto *propertyEditor = new PropertyEditor;
propertyEditor->setInspectedObject(slider);
propertyEditor->resize(420, 700);
propertyEditor->show();
```

この例で確認したいのは次の点です。

- プロパティーツリーが構築されるか
- `value` や `orientation` が表示されるか
- スライダー操作がエディター表示に反映されるか
- エディター側で変更した値がオブジェクトへ書き戻るか

# 9. デモの使い方

デモアプリは次のコマンドで起動できます。

```bash
./build/property_editor_demo
```

デモでは、左側に対象ウィジェット、右側に `PropertyEditor` が表示されます。

ここで主に確認できるのは次の内容です。

- 部品を操作したときの値追従
- enum や flags の編集 UI
- 色やカーソルなど型ごとのエディター
- フォント、サイズ、矩形、ロケールなどのサブプロパティー展開
- オブジェクトツリーからの選択切替
- オブジェクトツリー検索
- パンくず表示
- レイアウト詳細表示

# 10. オブジェクトツリー

`PropertyEditor` には、現在のルートオブジェクト配下を辿るオブジェクトツリー表示があります。

主な使い方は次のとおりです。

- `showObjectTree(rootObject)` を呼ぶ
- ツリー上のオブジェクト名をクリックして inspected object を切り替える
- `objectName` またはクラス名で検索欄から絞り込む
- 現在 inspected 中のオブジェクトはツリー側でも選択同期される

パンくずも表示され、現在選択中オブジェクトまでの親子パスを確認できます。
表示は長くなりすぎないように末尾優先で省略され、フルパスはツールチップで確認できます。

レイアウトを選択した場合は、次の情報も表示されます。

- `spacing`
- `contentsMargins`
- `sizeConstraint`
- 管理中アイテム数

# 11. Cling から使うときの流れ

Cling から使う場合は、基本的に次の順番になります。

1. `#pragma cling load(...)` で `property_editor.dylib` を読み込む
2. `#include "propertyeditor.h"` を読む
3. 観察対象の `QObject` または `QWidget` を作る
4. `PropertyEditor` を作って `setInspectedObject()` を呼ぶ
5. 対象とエディターを両方 `show()` する

最小例だけでなく、`examples/cling_demo.cpp`、`examples/demo_dialog.cpp`、
`examples/demo_colorwidget.cpp` も参考になります。

# 12. どのようなオブジェクトに向いているか

特に相性がよいのは次のような場面です。

- `QWidget` 派生クラスのプロパティー確認
- Cling で試作中のカスタム部品の挙動確認
- enum / flags / color / geometry などの編集確認
- dynamic property を含む状態確認

一方で、次のようなものはこのツールだけでは十分でない場合があります。

- `QObject` ではないデータ構造
- signal / slot の流れそのものの解析
- QML バインディング全体のデバッグ
- Qt Designer 相当のフォーム編集

# 13. 更新の仕組み

プロパティー表示が追従する理由は、主に次の 3 系統です。

- `QMetaProperty` の notify signal
- 主要ウィジェットの部品固有 signal
- 定期的な再読込

そのため、notify signal が無いプロパティーでも、一定範囲では表示更新が行われます。
ただし、更新の即時性や網羅性はプロパティーごとに異なります。

# 14. 注意点

- すべての Qt 型が専用エディターを持つわけではありません
- `qlonglong` / `qulonglong` / `unsigned int` は値域の都合で `QLineEdit` ベースです
- notify signal が無いプロパティーは timer 再読込に依存します
- レイアウト管理下の geometry 系プロパティーは編集制限される場合があります
- このツールは Qt Designer の内部 API 互換を目的としていません

# 15. 参考ファイル

使い方を把握するには、次のファイルを先に見るのが効率的です。

- `src/propertyeditor.h`
- `src/objectpropertyeditorwidget.h`
- `examples/cling_demo.cpp`
- `examples/demo_main.cpp`
- `examples/demo_widget_selector.h`
- `tests/objectpropertyeditorwidget_test.cpp`
