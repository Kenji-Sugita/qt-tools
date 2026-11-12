# ObjectSelector User Guide

## Overview

`objectselector` is a library for finding and selecting Qt `QObject` and `QWidget` instances.

The current distribution version is `1.0.0`, managed in the `VERSION` file.

It is primarily intended for:

- Selecting a `QWidget` by pointing to it directly on screen
- Selecting objects from an object tree
- Highlighting the selection target

This module handles selection. It does not include functionality for displaying
or editing the selected `QObject`.

The distribution `.zip` contains source code, not prebuilt binaries.

## Contents

- Shared library: `objectselector`
- Demo applications:
  - `objectpicker_demo`
  - `objecttree_demo`
- Tests:
  - `objectpicker_test`
  - `objecthighlighter_test`

Main classes:

- `ObjectPicker`
- `ObjectTreeWidget`
- `ObjectTreeDialog`
- `ObjectHighlighter`

## Build Prerequisites

You need:

- CMake 3.16 or later
- A C++17 compiler
- Qt 6
- The following Qt 6 modules:
  - `Core`
  - `Gui`
  - `Widgets`
  - `Test`

Set `CMAKE_PREFIX_PATH` if Qt is installed outside the standard search paths.

Example:

```bash
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos \
  -DBUILD_TESTING=ON
```

## Directory Structure

```text
objectselector/
  CMakeLists.txt
  USER_GUIDE.md
  src/
  tests/
  examples/
```

- `src/`: Library implementation
- `tests/`: Unit tests
- `examples/`: Demo applications

## Building

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
```

Omit `-DBUILD_TESTING=ON` if you do not want to build the tests.

## Installation

```bash
cmake --install build
```

The default installation prefix is `ICPP_PREFIX`, if specified; otherwise it is
the parent directory of the `icpp` executable found on PATH. If neither is
available, CMake's default is used.

To install elsewhere, set `CMAKE_INSTALL_PREFIX` during configuration.

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build
cmake --install build
```

Installed files are arranged as follows:

- `bin/`: Demo applications
- `include/`: Public headers
- `lib/`: Shared library

## Source Code Release ZIP

The distribution ZIP is a source code release. It does not include build
artifacts, installed files, or Git metadata.

```bash
unzip objectselector-1.0.0-source.zip
cd objectselector-1.0.0
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
```

To create a release, run the following from the repository root:

```bash
./release.sh
```

The resulting ZIP is `release/objectselector-<VERSION>-source.zip`.
`release.sh` uses a temporary staging directory and checks ZIP integrity.

## Build Outputs

The target name is `objectselector`.

The library filename depends on the operating system:

- macOS: `objectselector.dylib`
- Linux: `objectselector.so`
- Windows: `objectselector.dll`

Demo executables:

- `objectpicker_demo`
- `objecttree_demo`

## Running Tests

```bash
ctest --test-dir build --output-on-failure
```

## Running the Demos

```bash
./build/objectpicker_demo
./build/objecttree_demo
```

## Using ObjectPicker

Pass a visible root `QWidget` to `ObjectPicker`.

```cpp
auto *picker = new ObjectPicker(this);

connect(picker, &ObjectPicker::objectPicked,
        this, [this](QObject *targetObject) {
            // Use the selection result.
        });

picker->start(rootWidget);
```

### Basic Behavior

- `start(rootWidget)` enters picker mode.
- The candidate `QWidget` under the mouse is temporarily highlighted.
- Left-click to confirm the selection.
- Press `Esc` or right-click to cancel.
- Normal interaction with the target area is unavailable in picker mode.

### Cancellation

- Exiting with `Esc` or a right-click means that nothing was selected in this pick operation.
- `objectPicked` is not emitted.
- `selectedObject()` becomes `nullptr`.

### Managing the Confirmed Selection

`ObjectPicker` can receive a confirmed selection from outside the picker.

- `setSelectedObject(QObject *)`: Set the object to treat as the current selection.
- `selectedObject()`: Get the current confirmed selection.

Typically, use `setSelectedObject()` to reflect a selection made in a tree or another UI.

### Signals

- `objectPicked(QObject *targetObject)`: Emitted when a left-click confirms selection.
- `hoveredObjectChanged(QObject *targetObject)`: Emitted when the hovered candidate changes.
- `activeChanged(bool active)`: Emitted when picker mode starts or ends.

## Using ObjectTreeWidget

`ObjectTreeWidget` displays parent–child relationships between `QObject` instances as a tree.

```cpp
auto *treeWidget = new ObjectTreeWidget(this);
treeWidget->setRootObject(rootObject);
treeWidget->setCurrentObject(currentObject);

connect(treeWidget, &ObjectTreeWidget::objectActivated,
        this, [this](QObject *targetObject) {
            // Use the selection result.
        });
```

### Main Features

- Lists objects under the root object.
- Filters by object name and class name.
- Scrolls to the currently selected object.
- Shows the current location with breadcrumbs.

## Using ObjectTreeDialog

Use `ObjectTreeDialog` to display `ObjectTreeWidget` in a standalone dialog.

```cpp
auto *treeDialog = new ObjectTreeDialog(this);
treeDialog->setRootObject(rootObject);
treeDialog->setCurrentObject(currentObject);
treeDialog->show();
```

Receive selection notifications from its contained `ObjectTreeWidget`.

```cpp
connect(treeDialog->objectTreeWidget(), &ObjectTreeWidget::objectActivated,
        this, [this](QObject *targetObject) {
            // Use the selection result.
        });
```

## Using ObjectHighlighter

`ObjectHighlighter` highlights a supplied `QObject`.

```cpp
auto *highlighter = new ObjectHighlighter(this);
highlighter->setTargetObject(targetObject);
```

- A `QWidget` is highlighted with a filled rectangle.
- A `QLayout` is highlighted with an outline.
- `clear()` removes the highlight.

## Common Combinations

### Passing a Picker Selection to Another Widget

```cpp
connect(picker, &ObjectPicker::objectPicked,
        propertyEditor, &PropertyEditor::setInspectedObject);
```

### Synchronizing the Tree Selection with the Picker

```cpp
connect(treeDialog->objectTreeWidget(), &ObjectTreeWidget::objectActivated,
        this, [picker](QObject *targetObject) {
            picker->setSelectedObject(targetObject);
        });
```

## Notes

- `ObjectPicker` can directly select only visible `QWidget` instances.
- A `QLayout` cannot be picked directly on screen; select it from the tree.
- `hoveredObjectChanged` reports a temporary candidate, not a confirmed selection.
- Cancelling does not emit `objectPicked`.
- Qt itself is not included in the distribution.
- Depending on where Qt is installed, you may need to set `CMAKE_PREFIX_PATH`.
