# Cling Meta Property Editor (reimplementation)

Qt Designer の内部実装には依存せず、`QObject` / `QMetaObject` / `QMetaProperty` を使って
Cling から利用できる軽量プロパティーエディターを再実装した版です。

## 今回の再実装方針

以前の版では、変更通知の取り回しが不安定で、`QSlider::value` や `QComboBox::currentIndex`
などが確実に追従しないことがありました。

この版では、次の 3 段構えにしています。

1. `QMetaProperty::hasNotifySignal()` / `notifySignal()` があるプロパティーは notify signal に接続する
2. `QAbstractSlider` / `QComboBox` / `QLineEdit` / `QAbstractButton` など、よく使う部品は
   ウィジェット固有の signal にも直接接続する
3. notify signal が無い/拾い切れない場合の保険として、短い間隔の `QTimer` で再読込する

## 対応している代表的な型

- `bool`
- `int`
- `double`
- `QString`
- `QStringList`
- enum
- flags(QFlags)
- `QPoint`
- `QSize`
- `QRect`
- `QColor`
- `QByteArray`
- dynamic property

## ビルド

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos

cmake --build build
```

ルートの `VERSION` file が、このリポジトリの release version です。
`CMakeLists.txt` はこの値を読んで `project(property_editor VERSION ...)` を構成します。

## インストール

`property_editor` は、`icpp` と同じ install prefix に入れる前提です。
`cmake` configure 時に `CMAKE_INSTALL_PREFIX` を明示しなければ、`PATH` 上の `icpp`
を探し、その install prefix を既定値として使います。

たとえば `icpp` が `/usr/local/bin/icpp` にある場合は、`property_editor` も
`/usr/local` へ install されます。

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos

cmake --build build
cmake --install build
```

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

install される主なファイルは次のとおりです。

- `bin/property_editor_demo`
- `lib/property_editor.dylib` (`macOS`) / `lib/property_editor.so` (`Linux`)
- `include/objectpicker.h`
- `include/objecttreedialog.h`
- `include/objecttreewidget.h`
- `include/propertyeditor.h`
- `include/propertyeditorglobal.h`
- `VERSION`

この配置にすると、`icpp/bin/icpp` から見て `../lib/property_editor.*` と
`../include/*.h` が揃うため、`.inspect` で自動検出できます。

## source code release zip

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

## テスト

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DBUILD_TESTING=ON

cmake --build build
ctest --test-dir build --output-on-failure
```

追加した自動テストでは、主に次を確認します。

- dynamic property を含むプロパティーツリー構築
- bool プロパティーのチェックボックス編集
- int プロパティーの `QSpinBox` 編集
- `QSlider::orientation` のコンボボックス編集
- flags プロパティーの複数チェック編集
- `QWidget::cursor` のコンボボックス編集
- 外部変更時の `QSlider::value` 追従
- `QPoint` / `QSize` / `QRect` / `QStringList` / `QByteArray` の文字列変換

## デモ実行

```bash
./build/property_editor_demo
```

左側のスライダーやコンボボックスを操作すると、右側の `value` / `currentIndex` /
`currentText` が追従する想定です。

## Cling から使う例

`examples/cling_demo.cpp` を参考にしてください。

最小例は次の形です。

```cpp
#pragma cling load("/Users/sugita/src/qt/tools/propertyeditor/build/property_editor.dylib")
#include "propertyeditor.h"
#include <QVBoxLayout>
#include <QSlider>

auto *previewWindow = new QWidget;
auto *previewLayout = new QVBoxLayout(previewWindow);

auto *slider = new QSlider(Qt::Horizontal);
slider->setRange(0, 500);
previewLayout->addWidget(slider);
previewWindow->show();

auto *propertyEditorHost = new PropertyEditor;
propertyEditorHost->setInspectedObject(slider);
propertyEditorHost->resize(420, 700);
propertyEditorHost->show();
```

## 注意

- この環境では Qt 6.11.0 での実ビルド確認まではしていません。
- `QMetaProperty` の notify signal が無いプロパティーは、timer 再読込に依存します。
- `qlonglong` / `qulonglong` / `unsigned int` は、値域の都合で `QLineEdit` ベースにしています。
