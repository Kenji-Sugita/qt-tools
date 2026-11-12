# Cling Meta Property Editor (reimplementation)

A reimplementation of a lightweight property editor for Cling, using `QObject`,
`QMetaObject`, and `QMetaProperty` without depending on Qt Designer internals.

## Reimplementation Approach

In the previous version, change notifications were unreliable: properties such
as `QSlider::value` and `QComboBox::currentIndex` did not always stay in sync.

This version uses three layers:

1. Connect to the notify signal for properties with `QMetaProperty::hasNotifySignal()` / `notifySignal()`.
2. Also connect directly to widget-specific signals for common controls such as
   `QAbstractSlider`, `QComboBox`, `QLineEdit`, and `QAbstractButton`.
3. Reload at short intervals with a `QTimer` as a fallback when notify signals are absent or insufficient.

## Representative Supported Types

- `bool`
- `int`
- `double`
- `QString`
- `QStringList`
- enum
- flags (QFlags)
- `QPoint`
- `QSize`
- `QRect`
- `QColor`
- `QByteArray`
- Dynamic properties

## Build

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos

cmake --build build
```

The root `VERSION` file holds the release version for this repository.
`CMakeLists.txt` reads it to configure `project(property_editor VERSION ...)`.

## Installation

`property_editor` is intended to use the same installation prefix as `icpp`.
Unless you explicitly set `CMAKE_INSTALL_PREFIX` during CMake configuration,
it looks for `icpp` on `PATH` and uses its installation prefix as the default.

For example, if `icpp` is at `/usr/local/bin/icpp`, `property_editor` is also
installed under `/usr/local`.

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos

cmake --build build
cmake --install build
```

If `icpp` is unavailable or you want another location, set `CMAKE_INSTALL_PREFIX`.

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DCMAKE_INSTALL_PREFIX=$HOME/.local

cmake --build build
cmake --install build
```

Use `ICPP_PREFIX` to select a specific icpp installation instead of the one on `PATH`.

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DICPP_PREFIX=/custom/icpp/prefix
```

Main installed files:

- `bin/property_editor_demo`
- `lib/property_editor.dylib` (`macOS`) / `lib/property_editor.so` (`Linux`)
- `include/objectpicker.h`
- `include/objecttreedialog.h`
- `include/objecttreewidget.h`
- `include/propertyeditor.h`
- `include/propertyeditorglobal.h`
- `VERSION`

This layout places `../lib/property_editor.*` and `../include/*.h` relative to
`icpp/bin/icpp`, allowing `.inspect` to discover them automatically.

## Source Code Release ZIP

The distribution ZIP is a source code release. It contains neither prebuilt
binaries nor Qt itself.

- The root `VERSION` file manages the release version.
- Distribution ZIPs are placed in `release/`.
- ZIP filename: `propertyeditor-<version>-source.zip`
- ZIP top-level directory: `propertyeditor-<version>/`

Creation command:

```bash
./scripts/create_release_zip.sh
```

The current ZIP includes `CMakeLists.txt`, `VERSION`, `USER_GUIDE.md`, `src/`,
`examples/`, and `tests/`. It excludes `build/`, `install/`, `.DS_Store`, and temporary files.

## Tests

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DBUILD_TESTING=ON

cmake --build build
ctest --test-dir build --output-on-failure
```

The added automated tests primarily check:

- Building a property tree including dynamic properties
- Editing bool properties with checkboxes
- Editing int properties with `QSpinBox`
- Editing `QSlider::orientation` with a combo box
- Editing flags with multiple checkboxes
- Editing `QWidget::cursor` with a combo box
- Tracking external changes to `QSlider::value`
- String conversions for `QPoint`, `QSize`, `QRect`, `QStringList`, and `QByteArray`

## Running the Demo

```bash
./build/property_editor_demo
```

When you operate the sliders or combo boxes on the left, the `value`,
`currentIndex`, and `currentText` values on the right should update accordingly.

## Example Usage from Cling

See `examples/cling_demo.cpp`.

A minimal example:

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

## Notes

- An actual build with Qt 6.11.0 has not been verified in this environment.
- Properties without a `QMetaProperty` notify signal rely on timer-based reloads.
- `qlonglong`, `qulonglong`, and `unsigned int` use `QLineEdit` because of their value ranges.
