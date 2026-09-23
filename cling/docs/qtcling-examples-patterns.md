# qtcling examples pattern survey

## Scope

This note summarizes the current `examples/` tree from the perspective of a
future `qtcling.toml`, `qtcling-project`, and autogen workflow.

The goal is not to document every example individually. The goal is to identify
the project patterns that qtcling must hide from users when there is no normal
build system.

## Inventory

Approximate file counts in `examples/`:

```text
687 .cpp
213 .h
222 .pro
225 CMakeLists.txt
129 startup.cpp
44  .qrc
23  .ui
21  .moc
9   .ts
9   .qm
1   .json
1   .pri
```

The tree has both qmake and CMake project metadata. It also has many hand-written
`startup.cpp` files used by qtcling-style loading.

## qmake patterns

Across 222 `.pro` files:

```text
QT                 185 projects
SOURCES            188 projects
HEADERS            122 projects
FORMS               14 projects
RESOURCES           42 projects
TRANSLATIONS         9 projects
INCLUDEPATH         14 projects
DEFINES              2 projects
CONFIG              20 projects
SUBDIRS             32 projects
```

Qt modules seen in `.pro` files:

```text
widgets       177
network         7
concurrent      3
printsupport    2
gui-private     2
core            1
gui             1
```

Removed modules:

```text
QT -= gui     5
```

Important `CONFIG` values:

```text
cmdline       10
app_bundle     3
console        3
c++20          1
plugin         1
debug          1
ordered        1
```

## CMake patterns

Across 225 `CMakeLists.txt` files:

```text
qt_add_executable          187
qt_standard_project_setup   87
qt_add_resources            30+
qt_add_translations          8+
AUTOMOC                    101
AUTOUIC                    100
AUTORCC                    100
qt_generate_deploy_app_script 73
qt_add_plugin                1
```

The CMake side confirms the same core set:

- source/header lists
- Qt module selection
- automatic moc/uic/rcc
- resources
- translations
- deploy scripts
- occasional plugin target

## startup.cpp patterns

There are 129 `startup.cpp` files.

Quoted include patterns inside them:

```text
source .cpp       282
moc_*.cpp         118
qrc_*.cpp          33
other quoted        5
```

Include count distribution:

```text
1 include   33 files
2 includes   9 files
3 includes  39 files
4 includes  13 files
5 includes  21 files
6 includes   5 files
7 includes   2 files
8 includes   3 files
9 includes   1 file
10 includes  2 files
16 includes  1 file
```

Examples with larger startup wrappers:

- `examples/widgets/itemviews/addressbook/startup.cpp`
  - 16 includes
  - includes `.h` files directly as well as `.cpp`, `moc_*.cpp`, and `main.cpp`
- `examples/widgets/painting/affine/startup.cpp`
  - 10 includes
  - includes files from `../shared`
  - includes shared generated files and resources
- `examples/widgets/graphicsview/diagramscene/startup.cpp`
  - 10 includes
  - several sources, moc files, resource file, and `main.cpp`
- `examples/widgets/itemviews/spreadsheet/startup.cpp`
  - 9 includes
  - several sources, moc files, resource file, and `main.cpp`

Implication: automatic startup generation cannot rely only on immediate sibling
`.cpp` files forever. It needs either project metadata, `qtcling.toml`, or a
small parser for `.pro` / `CMakeLists.txt`.

## Concrete feature groups

### Basic Widgets examples

Common shape:

```text
QT += widgets
SOURCES += main.cpp widget.cpp
HEADERS += widget.h
startup.cpp includes widget.cpp, moc_widget.cpp, main.cpp
```

Required qtcling support:

- modules: `Core Gui Widgets`
- scan `Q_OBJECT`
- run `moc`
- generate startup wrapper

### UI form examples

Examples include:

- `examples/hands-on/colorwidget_step*`
- `examples/hands-on/formcreationpractice`
- `examples/widgets/tutorials/notepad`
- `examples/widgets/animation/easing`
- `examples/widgets/itemviews/editabletreemodel`
- object-browser examples under `cpp-advanced`

Required qtcling support:

- `forms`
- run `uic`
- add generated `ui_*.h` include directory
- include generated headers before sources that reference them

### Resource examples

There are 42 `.pro` files with `RESOURCES`.

Required qtcling support:

- `resources`
- run `rcc`
- include `qrc_*.cpp` in startup wrapper
- handle resources used by `:/...` paths

### Translation examples

There are 9 `.pro` files with `TRANSLATIONS`.

Examples include:

- `examples/hands-on/colorwidget_step2` through `step6`
- `examples/cpp-basics/examples/dynamictranslation`
- `examples/cpp-basics/examples/resources`
- `examples/cpp-basics/examples/paintprogram/step2`
- `examples/cpp-basics/solutions/colorwidget`

Required qtcling support:

- `translations`
- stale `lrelease`
- opt-in `lupdate`
- regenerate resources if `.qm` files are embedded through `.qrc`

### Non-Widgets / console examples

Examples include:

- `QT -= gui`
- `CONFIG += cmdline`
- `QCoreApplication`

Required qtcling support:

- project `type = "console"` or `type = "core"`
- modules can be just `Core`, or `Core Network`, or `Core Concurrent`
- startup wrapper still useful, but `QApplication` startup file is not always
  the right default

### Extra module examples

Examples require more than Widgets:

```text
Network       7
Concurrent    3
PrintSupport  2
gui-private   2
```

Required qtcling support:

- `modules = [...]`
- module-to-include/library mapping
- `--explain` output for unsupported/private modules

`gui-private` should be treated as advanced/unsupported at first.

### Include path and define examples

Examples use:

```text
INCLUDEPATH   14
DEFINES        2
CONFIG += c++20 1
```

Required qtcling support:

- `include_dirs`
- `defines`
- `std`

This is necessary before examples with shared sibling directories or compile
feature switches can be handled consistently.

### Plugin example

There is at least one plugin-oriented example:

- `examples/cpp-advanced/solutions/sif`
  - `CONFIG += plugin`
  - `Q_PLUGIN_METADATA(... FILE "sif.json")`

Initial qtcling behavior should detect this and report unsupported or advanced.
It should not block the basic Widgets workflow.

This should be treated as a much later topic. A normal Qt dynamic plugin is a
shared library file with metadata and loader-visible entry points. Cling JIT
code is not such a file, and static plugins cannot be added after process
startup in the same way as normal source snippets.

For MVP:

- detect `CONFIG += plugin`, `qt_add_plugin`, or `Q_PLUGIN_METADATA`
- report plugin projects as unsupported in `qtcling-project explain`
- do not attempt to autogenerate or run plugin examples

Possible future options:

- load an already-built plugin file through `QPluginLoader`
- call an external build step to build the plugin as a real shared library
- use REPL-created QObject implementations directly when plugin loading is not
  the actual point of the experiment

## Recommended qtcling.toml MVP

The minimal schema should be slightly larger than the first draft:

```toml
[project]
name = "example"
type = "widgets"       # widgets | console | core
main = "main.cpp"
startup = "auto"

sources = []
headers = []
forms = []
resources = []
translations = []

modules = ["Core", "Gui", "Widgets"]
include_dirs = ["."]
defines = []
std = "c++2c"

[autogen]
enabled = true
output_dir = ".qtcling/autogen"
lupdate = false
lrelease = "stale"
```

`qtcling-project` should also account for the separate `icpp` tool tree under
`/Users/sugita/src/tools/icpp`, but only as a design constraint for now. As of
2026-05-18, that tree does not contain a `.toml` file; persistent user options
such as `.quiet`, `.newname`, and `.lang` are stored through QSettings, while
external tool paths use `ICPP_*` environment variables. The `icpp` TOML idea is
still conceptual, so `qtcling-project` should not assume an implemented `icpp`
TOML format.

The role of `icpp` is broader than configuration. Even without any `.toml`
file, `icpp` is already a safer and more usable layer over direct `qtcling`
usage. Its most important benefit is crash containment. `qtcling` / Cling can
crash while evaluating user code; `icpp` can keep the surrounding workflow alive
by running the interpreter as a separate process and preserving editor / session
state.

`qtcling-project` itself does not provide crash containment. It is a project
metadata and autogen tool, so it cannot protect a running `qtcling` / Cling
process from crashes. Crash handling belongs in `icpp` or another process
supervision layer. The only requirement for `qtcling-project` is to keep its
metadata and generated-file layout easy for such a layer to consume.

This weakens the role of `qtcling-project` enough that further improvement is
now stopped. It should not be treated as a major user-facing workflow unless it
solves something that `icpp` does not already solve better. Keep the existing
`explain` prototype for reference, but do not expand `qtcling-project prepare`
or turn `qtcling.toml` into the main workflow. Future project/autogen workflow
should be considered on the `icpp` side first.

If `icpp` later grows a TOML project/config file, `qtcling.toml` must avoid
accidental schema, key, and autogen-directory conflicts. Keep `icpp.toml` and
`qtcling.toml` separate unless a shared schema is designed deliberately.

This covers:

- `wiggly`
- colorwidget steps
- formcreationpractice
- resource examples
- translation examples
- network/printsupport/concurrent examples, once module mapping exists
- console/core examples, once `type` is respected

## MVP support tiers

### Tier 1: first implementation

- explicit `qtcling.toml`
- `modules`
- `include_dirs`
- `defines`
- `std`
- `sources`
- `headers`
- `forms`
- `resources`
- `translations`
- `startup = "auto"`
- `moc`
- `uic`
- `rcc`
- stale `lrelease`
- no default `lupdate`
- generated files in `.qtcling/autogen`
- `qtcling-project explain`
- `qtcling-project prepare` (stopped; design note only)

### Tier 2: use existing project metadata

- parse simple `.pro`
- parse simple `CMakeLists.txt`
- generate initial `qtcling.toml` (stopped; design note only)
- support parent/shared source directories
- better source ordering
- existing `startup.cpp` import / comparison

### Tier 3: advanced Qt features

- private Qt modules
- plugins
- QML
- deployment/package
- D-Bus
- Remote Objects
- ShaderTools

## Conclusion

The examples confirm that `qtcling-project` should not be only a wrapper around
`run_moc`, `run_uic`, and `run_rcc`.

It needs a small project model:

- source/header/form/resource/translation lists
- Qt modules
- include paths
- defines
- language standard
- app type
- startup wrapper policy

That project model is also the right bridge to a future Qt Creator plugin.
