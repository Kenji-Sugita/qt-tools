<!--
genpdf --title 'qmlprop User Guide' --autho='SRA, Inc.' qmlprop_user_guide.md
-->

# 1. What Is qmlprop?

`qmlprop` is a command-line tool that instantiates QML types and displays the information visible on those types.

Typical uses include:

- Inspecting the properties, methods, and enumerators of a QML type
- Checking APIs including inherited members
- Checking default properties and methods
- Checking attached properties
- Distinguishing types with the same name in different modules
- Investigating why a type cannot be instantiated

---

# 2. Features

`qmlprop` instantiates the target type in QML and displays the following through `QObject` and `QMetaObject`:

- Module name
- Type flags
  - `creatable`
  - `uncreatable`
  - `singleton`
  - `attached provider`
  - `value type` (for listings/failure diagnostics)
- C++ class hierarchy
- Default property
- Default method
- Property list
  - Type
  - Current value
  - Attributes
    - `readonly`
    - `final`
    - `constant`
    - `required`
    - `bindable`
    - `default`
    - `deferred`
    - `list`
    - `object`
    - `override`
    - `inherited`
- Method list
- Enumerator list
- The declaring C++ class of each member
- Attached properties
- All candidates sharing the same type name
- Classification of instantiation failures

---

# 3. Building

```bash
cmake -S . -B build
cmake --build build
```

Notes:

- A Qt 6 development environment is required.
- The QML modules you want to inspect must be installed in the runtime environment.
- Types may fail to instantiate if additional modules are missing.

---

# 4. Basic Usage

## 4.1 Inspect One Type

```bash
qmlprop Rectangle
```

Example:

```text
Qt 6.11.0
Type: Rectangle
Module: QtQuick
Type flags: creatable
Class: QQuickRectangle -> QQuickItem -> QObject
Default property: data

Properties:
    color: QColor = "#ffffffff"
    radius: double = 0
    ...
```

## 4.2 Include Inherited Members

```bash
qmlprop --all Item
```

Without `--all`, the output primarily shows members declared by the type itself. With `--all`, it also includes members inherited from base classes.

## 4.3 Show Debug Information

```bash
qmlprop --debug FolderDialog
qmlprop -dd FolderDialog
```

You can repeat `--debug`:

- `-d` / `--debug`: Shows the selected import profile and the QML source used for instantiation.
- `-dd`: Shows more detailed failure logs.

## 4.4 Show Attached Properties

```bash
qmlprop --show-attached Item
```

## 4.5 Show Declaring Classes

```bash
qmlprop --show-declared-in Item
```

This appends `[declared in ...]` to each property, method, and enumerator.

## 4.6 List Available Type Names

```bash
qmlprop --list-types
```

The listing includes each type's module and flags.

## 4.7 List Available Modules

```bash
qmlprop --list-modules
```

This lists the QML module URIs visible in the current runtime environment.

## 4.8 Show an Index of a Module

```bash
qmlprop --module-index QtQuick.Controls
```

This displays one row per type in the specified module. The main columns are:

- `Type`: Type name
- `Base`: Base class name
- `Flags`: Type flags
- `Props`: Number of properties declared by the type itself
- `Methods`: Number of public/protected methods visible on the type itself
- `Enums`: Number of enumerators declared by the type itself
- `Default`: Default property name
- `Status`: `ok` / `failed` / `no profile`

Notes:

- This indexes QML types visible in the module. Modules focused on C++ APIs may have only one entry or very few entries.
- For example, in this environment, `QtNetwork` exposes only `NetworkInformation` as a QML type, so its index has one row.
- `Status: no profile` means the type catalog contains a type from that module, but `qmlprop` does not yet have an import profile for trying that module.

---

# 5. Command-Line Options

## 5.1 Option List

| Option | Meaning |
|---|---|
| `-a`, `--all` | Include inherited members. |
| `-l`, `--list-types` | List importable QML type names visible in the current runtime environment. |
| `-m`, `--list-modules` | List QML module URIs visible in the current runtime environment. |
| `--module-index <module>` | Display the specified QML module's types in a table with one row per type. |
| `-d`, `--debug` | Show debug information; repeat for more detail. |
| `--show-attached` | Show attached properties. |
| `--show-declared-in` | Show each member's declaring C++ class. |
| `--all-matches` | Show all types with the same name, grouped by module. |
| `--no-pager` | Write directly to standard output without a pager. |
| `-h`, `--help` | Show help. |
| `-v`, `--version` | Show the version. |

## 5.2 Positional Argument

```bash
qmlprop [options] <type>
```

`<type>` is a QML type name.

Examples:

- `Rectangle`
- `Item`
- `FolderDialog`
- `Component`
- `T.Control`
- `QLabP.FolderDialog`

---

# 6. Reading the Output

## 6.1 Report Layout

A normal type report generally appears in this order:

1. Qt version, such as `Qt 6.11.0`
2. `Type:`
3. `Module:`
4. `Type flags:`
5. `Class:`
6. `Default property:` / `Default method:`
7. `Properties:`
8. `Attached:`
9. `Methods:`
10. `Enumerators:`

## 6.2 `Module:`

`Module:` shows the inferred module, prioritizing the import profile that actually instantiated the type successfully.

When several modules have a type with the same name, the report prioritizes the **import configuration that actually worked**, rather than just matching names.

## 6.3 `Type flags:`

Common flags:

- `creatable`: The type is expected to be instantiable as a normal QML object.
- `uncreatable`: The type is registered but normally cannot be instantiated directly.
- `singleton`: A singleton type.
- `attached provider`: A type that provides an attached type.
- `value type`: A value type. It is not `QObject`-based and is not a normal object-instantiation target.

## 6.4 `Class:`

This is the C++ class hierarchy actually visible at runtime. Internal QML wrapper types are excluded where possible to make the underlying hierarchy clear.

## 6.5 `Properties:`

Each property line shows information such as:

```text
text: QString = "Hello"
width: double = 100
visible: bool = true
```

Attributes are included where appropriate:

```text
model: QObject* = null readonly
status: QQmlComponent::Status -- readonly
delegate: QQmlComponent* = null default object
```

## 6.6 `Methods:` / `Enumerators:`

Methods include their signatures. Enumerators show keys and numeric values.

---

# 7. Finding Types

## 7.1 Start with the Type Name

```bash
qmlprop Label
```

Internally, `qmlprop` tries multiple import profiles in sequence and displays the first type it successfully instantiates.

## 7.2 Use Qualified Names

Some types can be specified with qualified names:

```bash
qmlprop T.Control
qmlprop QLabP.FolderDialog
```

The main aliases currently handled specially are:

- `T` → `QtQuick.Templates`
- `QLabP` → `Qt.labs.platform`

## 7.3 Find Candidates with `--list-types`

```bash
qmlprop --list-types | grep Label
```

You may find multiple types with the same name.

---

# 8. Multiple Types with the Same Name

## 8.1 Default Behavior

When multiple modules contain a type with the same name, `qmlprop` tries import profiles in sequence and displays **the first one that instantiates successfully**.

## 8.2 Show All Candidates

```bash
qmlprop --all-matches Label
```

Example:

```text
== Match 1/7: Qt.labs.StyleKit::Label [creatable] ==
No import profile is available for module: Qt.labs.StyleKit

== Match 2/7: QtQuick.Controls.Basic::Label [creatable] ==
No import profile is available for module: QtQuick.Controls.Basic

== Match 3/7: QtQuick.Templates::Label [creatable] ==
Qt 6.11.0
Type: Label
Module: QtQuick.Templates
Type flags: creatable
Class: QQuickLabel -> QQuickText -> QQuickImplicitSizeItem -> QQuickItem -> QObject
...
```

A listed candidate **has not necessarily been tried**.

- `No import profile is available for module: ...`
  - The type candidate was found in the type catalog.
  - However, the current `qmlprop` has no import profile for that module.
  - The candidate was therefore skipped.

- If a type report follows:
  - The candidate was successfully instantiated.
  - `qmlprop` displays its normal report.

Notes:

- The current `--all-matches` does not display every candidate's failure diagnostics.
- Candidates without import profiles are shown, but **candidates that were tried and failed to instantiate are silently skipped**.
- The number of reports from `--all-matches` may therefore differ from the total number of candidates in the type catalog.

## 8.3 Inspecting `Label` from Qt Quick Controls

In many environments, use:

```bash
qmlprop Label
qmlprop --debug Label
```

`--debug` shows the selected import profile. If it reports:

```text
Selected profile: QtQuick.Controls
```

the displayed `Label` is the one from `QtQuick.Controls`.

---

# 9. Instantiation Failure Diagnostics

When a type cannot be instantiated, the tool can classify the reason instead of only reporting `could not be instantiated`.

Example:

```text
SomeType could not be instantiated.
Reason: required property not set
Details: Required property 'model' is not initialized.
```

## 9.1 Common Failure Reasons

| `Reason:` | Meaning |
|---|---|
| `required property not set` | Instantiation failed because a required property was not set. |
| `value type` | A value type that cannot be instantiated as a `QObject`-based QML object. |
| `abstract type` | An abstract type that cannot be instantiated directly. |
| `attached-only type` | A type intended only for use as an attached type. |
| `module import missing` | The tried import configurations could not resolve the type name. A required module may be missing. |
| `dependent type missing` | Another required type or module may be missing. |
| `uncreatable type` | The type registration was found, but the type cannot be instantiated as a normal QML object. |
| `unknown` | A failure not covered above. Use `--debug` for details. |

## 9.2 Combine with `--debug`

```bash
qmlprop --debug Component
qmlprop -dd Component
```

Useful information for detailed investigation includes:

- The list of import profiles tried
- The QML source actually used for instantiation
- `QQmlError` details
- `Reason:` and `Details:`

---

# 10. Understanding Import Profiles

`qmlprop` does not stop as soon as it finds a type name. It tries multiple import configurations in sequence to verify instantiation.

Representative profiles include:

- Base modules
  - `QtCore`
  - `QtQml`
  - `QtQuick`
- `QtQuick.Controls`
- `QtQuick.Templates`
- `QtQuick.Dialogs`
- `QtQuick.Pdf`
- `QtQuick.Shapes`
- `QtQuick.Layouts`
- `QtQuick.Window`
- `QtQuick3D`
- `QtLocation`
- `QtMultimedia`
- `QtWebEngine`
- `Qt.labs.platform`
- `Qt.labs.settings`
- Other supported modules

Notes:

- Even a type visible with `--list-types` cannot be tried directly if `qmlprop` lacks a corresponding import profile.
- Implementation-specific or internal module candidates may appear.
- `--all-matches` is useful for finding candidates, but it also lists candidates that were found and are not yet supported.

## 10.1 Pager Behavior

When standard output is connected to a terminal, normal type reports use a pager by default.

- This makes long output easier to read in the terminal.
- When piping or redirecting, output automatically goes directly to standard output.
- Use `--no-pager` to disable the pager explicitly.

```bash
qmlprop --no-pager Rectangle
qmlprop --list-types --no-pager
```

---

# 11. Common Examples

## 11.1 Basic Qt Quick Types

```bash
qmlprop Rectangle
qmlprop Item
qmlprop --all Item
```

## 11.2 Controls

```bash
qmlprop Button
qmlprop Label
qmlprop --debug Label
```

## 11.3 Dialogs / labs.platform

```bash
qmlprop FolderDialog
qmlprop QLabP.FolderDialog
```

## 11.4 Templates Alias

```bash
qmlprop T.Control
```

## 11.5 Search the Type List

```bash
qmlprop --list-types | grep Dialog
qmlprop --list-types | grep Pdf
```

## 11.6 Check All Types with the Same Name

```bash
qmlprop --all-matches Label
qmlprop --all-matches --debug Label
```

---

# 12. Troubleshooting

## 12.1 `could not be instantiated`

Check in this order:

1. Add `--debug`.
2. Check `Reason:` and `Details:`.
3. Check whether the type requires any required properties.
4. Check whether it is a value type, abstract type, or attached-only type.
5. Check whether the necessary QML modules are installed.
6. Check for missing dependent modules.

## 12.2 `No import profile is available for module: ...`

This means the candidate module was found, but the current `qmlprop` has no import profile for trying it.

Possible actions:

- Check whether another existing candidate provides the desired type.
- Use `--debug` to see which profile was used.
- If needed, add an import profile for that module to `qmlprop`.

## 12.3 `Module:` Is Not What You Expected

When several types share a name, matching only the name can be misleading. The current `qmlprop` determines `Module:` by **prioritizing the successful import profile**.

Check with:

```bash
qmlprop --debug <type>
```

## 12.4 Inspecting Value Types

Value types cannot be instantiated as normal `QObject`-based QML objects. They are not normal type report targets and do not appear in `--list-types`. However, failure diagnostics may classify a failed instantiation as a value type.

---

# 13. Limitations

- Types from QML modules not installed in the environment cannot be instantiated.
- Not every listed type has a corresponding import profile.
- Types requiring required properties fail to instantiate without those properties.
- Value types, abstract types, and attached-only types cannot be displayed as normal objects.
- Some types need import aliases or additional dependency imports.
- `--all-matches` is useful for finding candidates, but internal or unsupported modules may appear.
- `--all-matches` displays candidates lacking import profiles, but does not provide individual reports for candidates whose instantiation failed.

---

# 14. Exit Status

- Success: `0`
- Failure: `1`

Scripts can use these exit codes to determine success or failure.

---

# 15. Commands to Learn First

If unsure where to start, these five commands are enough:

```bash
qmlprop Rectangle
qmlprop --all Item
qmlprop --debug Label
qmlprop --show-attached Item
qmlprop --all-matches Label
```

They provide a starting point for most investigations.
