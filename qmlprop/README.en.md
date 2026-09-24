# qmlprop

A command-line program that displays the properties, methods, and enumerators of QML types.

## Main Changes

- Removed Qt 5 support.
- Reorganized the code for Qt 6 and updated the configuration for Qt 6.11.0.
- Migrated from a qmake `.pro` file to CMake.
- Removed private header dependencies.
- Replaced the large, fixed QML template with an approach that tries candidate modules in order.
- Split responsibilities previously concentrated in `main.cpp`.
- Added the module, type flags, default property, and declaring class to type reports.
- Added creatability, singleton, and attached provider flags to `--list-types`.

## Build

```bash
cmake -S . -B build
cmake --build build
```

A shared-library build of Qt 6 is assumed. To inspect additional QML modules,
those modules must be installed in your Qt environment.

## Usage

```bash
qmlprop Rectangle
qmlprop --all Item
qmlprop --debug FolderDialog
qmlprop --show-attached Item
qmlprop --all-matches Button
qmlprop --list-types
qmlprop --list-modules
qmlprop --module-index QtQuick.Controls
```

## Report Contents

A normal type report displays:

- The module to which the type belongs
- Type flags, such as `creatable`, `singleton`, and `attached provider`
- The C++ class hierarchy
- The default property and default method
- Property values
- Property attributes: `readonly`, `final`, `constant`, `required`, `bindable`,
  `default`, `deferred`, `list`, and `object`
- The declaring class of each property, method, and enumerator
- Numeric enumerator values

## Notes

- Some types cannot be created unless the required QML modules are installed.
- When multiple modules contain types with the same name, the module is normally
  inferred from the first import configuration that successfully creates the type.
- `--all-matches` displays types with the same name for each module in turn.
- Qualified names are also supported, for example `T.Control` and `QLabP.FolderDialog`.
- `--list-types` scans `qmldir` and `*.qmltypes`, excluding non-`QObject` types
  such as value and sequence types.
- `--list-modules` lists QML module URIs visible in the current runtime environment.
- `--module-index <module>` displays a table with one type per row for the specified
  module. This is an index of QML types; modules focused on C++ APIs may have few entries.
- The `Attached:` section is shown only when `--show-attached` is specified.

Additional options:

- `--show-declared-in` adds a `[declared in ...]` suffix to properties, methods,
  enumerators, and default members.

## Pager

When standard output is a terminal, qmlprop sends normal output through a pager
by default. This makes long type reports easier to read without explicitly
piping them to `less`.

Use `--no-pager` to write directly to standard output. Redirected output and
output piped to another command automatically bypass the pager.
