---
genpdf:
  Format: book
  Title: Property Editor User Guide
  Subtitle: Version 1.0
  Author: SRA, Inc.
---

# 1. Overview

`property_editor` displays and edits object properties using `QObject`, `QMetaObject`, and `QMetaProperty`, without depending on Qt Designer internals.

This guide goes beyond build instructions to explain how to **use `PropertyEditor` from Cling to inspect and edit Qt objects**.

Keep the following prerequisites in mind:

- The target must be a `QObject`-based object.
- Displayed information comes from `QMetaObject` and dynamic properties.
- Whether a property can be edited depends on its type and writable attribute.

`property_editor` is best used as a **lightweight property editor for quickly trying changes while inspecting Qt state in Cling**, rather than as a complete replacement for Qt Designer.

# 2. Features

Connecting `PropertyEditor` to a target object provides:

- A property list based on `QMetaProperty`
- Display of dynamic properties
- Value editing with editors suited to each type
- Display updates through notify signals and widget-specific signals
- Timer-based reloading when needed

Representative supported types include:

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
- dynamic properties

# 3. Basic Approach

Instead of treating every Qt property identically, the tool updates and edits properties as follows:

1. Prefer notify signals when `QMetaProperty::hasNotifySignal()` is available.
2. Also connect to widget-specific signals for common widgets such as `QSlider` and `QComboBox`.
3. Use a timer as a fallback when notify signals are missing or do not cover all changes.

The design therefore supports following changes made to widgets from Cling, as well as displaying their values.

# 4. Building

A typical build is:

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos

cmake --build build
```

The main generated files are:

- `build/property_editor.dylib`
- `build/property_editor_demo`

The release version is maintained in the root `VERSION` file.
`CMakeLists.txt` reads it to configure `project(property_editor VERSION ...)`.

# 5. Installation

`property_editor` is intended to be installed under the same prefix as `icpp`, for use through the `.inspect` command in `icpp`.

If `CMAKE_INSTALL_PREFIX` is not explicitly specified during CMake configuration, the build looks for `icpp` on `PATH` and uses its installation prefix by default. For example, if `icpp` is at `/usr/local/bin/icpp`, `property_editor` is also installed under `/usr/local`.

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos

cmake --build build
cmake --install build
```

The main installation locations are:

- `bin/property_editor_demo`
- `lib/property_editor.dylib` (`macOS`) / `lib/property_editor.so` (`Linux`)
- `include/objectpicker.h`
- `include/objecttreedialog.h`
- `include/objecttreewidget.h`
- `include/propertyeditor.h`
- `include/propertyeditorglobal.h`
- `VERSION`

This layout places `../lib/property_editor.*` and `../include/*.h` relative to `icpp/bin/icpp`, allowing `.inspect` to detect them automatically.

Specify `CMAKE_INSTALL_PREFIX` explicitly if `icpp` is unavailable or you want another installation location.

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DCMAKE_INSTALL_PREFIX=$HOME/.local

cmake --build build
cmake --install build
```

To use a specific `icpp` installation instead of the one on `PATH`, override it with `ICPP_PREFIX`:

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DICPP_PREFIX=/custom/icpp/prefix
```

# 6. Source Code Release ZIP

The distribution ZIP is a source code release. It does not include prebuilt binaries or Qt itself.

- The release version is maintained in the root `VERSION` file.
- Distribution ZIPs are placed in `release/`.
- The ZIP filename is `propertyeditor-<version>-source.zip`.
- The top-level directory inside the ZIP is `propertyeditor-<version>/`.

Create it with:

```bash
./scripts/create_release_zip.sh
```

The current ZIP includes `CMakeLists.txt`, `VERSION`, `USER_GUIDE.md`, `src/`, `examples/`, and `tests/`. It excludes `build/`, `install/`, `.DS_Store`, and temporary files.

# 7. Testing

To build with tests:

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DBUILD_TESTING=ON

cmake --build build
ctest --test-dir build --output-on-failure
```

The automated tests primarily check:

- Property tree construction, including dynamic properties
- Checkbox editing of bool properties
- `QSpinBox` editing of int properties
- enum / flags editing
- Combo box editing of `QWidget::cursor`
- Value updates after external changes
- String conversion for `QPoint`, `QSize`, `QRect`, `QStringList`, and `QByteArray`

# 8. A Minimal Example to Try First

The simplest usage is to create the `QObject` you want to inspect and pass it to `PropertyEditor`.

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

Use this example to check that:

- The property tree is constructed.
- `value` and `orientation` are displayed.
- Moving the slider updates the editor display.
- Values changed in the editor are written back to the object.

# 9. Using the Demo

Start the demo application with:

```bash
./build/property_editor_demo
```

The demo shows the target widget on the left and `PropertyEditor` on the right.

It lets you check:

- Value updates when interacting with widgets
- The enum and flags editing UI
- Type-specific editors, such as color and cursor editors
- Expansion of subproperties for fonts, sizes, rectangles, locales, and more
- Changing the selection through the object tree
- Object tree search
- Breadcrumb display
- Layout details

# 10. Object Tree

`PropertyEditor` provides an object tree for browsing the descendants of the current root object.

Basic usage:

- Call `showObjectTree(rootObject)`.
- Click an object name in the tree to change the inspected object.
- Filter by `objectName` or class name using the search field.
- The tree selection stays synchronized with the currently inspected object.

Breadcrumbs show the parent-child path to the selected object. Long paths are shortened while preserving the trailing portion; the full path is available in a tooltip.

Selecting a layout also displays:

- `spacing`
- `contentsMargins`
- `sizeConstraint`
- Number of managed items

# 11. Workflow from Cling

The usual sequence when using Cling is:

1. Load `property_editor.dylib` with `#pragma cling load(...)`.
2. Include `#include "propertyeditor.h"`.
3. Create the `QObject` or `QWidget` to inspect.
4. Create `PropertyEditor` and call `setInspectedObject()`.
5. Call `show()` on both the target and the editor.

In addition to the minimal example, see `examples/cling_demo.cpp`, `examples/demo_dialog.cpp`, and `examples/demo_colorwidget.cpp`.

# 12. Suitable Objects and Use Cases

The tool is particularly useful for:

- Inspecting properties of `QWidget` subclasses
- Checking custom widgets being prototyped in Cling
- Trying edits to enum, flags, color, geometry, and similar properties
- Inspecting state that includes dynamic properties

The tool alone may not be sufficient for:

- Data structures that are not `QObject`-based
- Analyzing signal/slot flow itself
- Debugging an entire set of QML bindings
- Form editing comparable to Qt Designer

# 13. How Updates Work

The property display follows changes through three main mechanisms:

- `QMetaProperty` notify signals
- Widget-specific signals for common widgets
- Periodic reloading

This allows some properties without notify signals to update as well. However, update immediacy and coverage vary by property.

# 14. Notes and Limitations

- Not every Qt type has a dedicated editor.
- `qlonglong`, `qulonglong`, and `unsigned int` use `QLineEdit`-based editors because of their value ranges.
- Properties without notify signals depend on timer-based reloading.
- Editing geometry-related properties may be restricted when a layout manages the object.
- Compatibility with Qt Designer's internal APIs is not a goal.

# 15. Reference Files

Start with these files to understand usage:

- `src/propertyeditor.h`
- `src/objectpropertyeditorwidget.h`
- `examples/cling_demo.cpp`
- `examples/demo_main.cpp`
- `examples/demo_widget_selector.h`
- `tests/objectpropertyeditorwidget_test.cpp`
