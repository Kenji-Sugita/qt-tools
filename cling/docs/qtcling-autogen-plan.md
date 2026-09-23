# qtcling autogen plan

## Position

This is a future plan.

qtcling does not use a normal build system when users experiment from the REPL.
Therefore Qt build steps such as `moc`, `uic`, `rcc`, and the current
`startup.cpp` wrapper should eventually be hidden from the user.

The goal is that users can focus on Qt/C++ code, not on generated files.

## Current situation

The repository currently has helper commands:

- `run_moc`
- `run_uic`
- `run_rcc`
- `run_all`

These are useful for experiments, but they still expose build-system concepts.

See also `docs/qtcling-examples-patterns.md` for a survey of the current
`examples/` tree and the patterns that affect `qtcling.toml` design.

Many examples also contain generated files:

- `moc_*.cpp`
- `*.moc`
- `ui_*.h`
- `qrc_*.cpp`

Many examples use `startup.cpp` to include source files and generated files,
then call `main(argc, argv)` through a simple `startup()` wrapper.

Example shape:

```cpp
#include "widget.cpp"
#include "moc_widget.cpp"
#include "qrc_resources.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
```

This works, but it is still something users should not have to write.

## Desired user experience

Minimum target:

```sh
cd examples/wiggly
qtcling
```

Then in the REPL:

```cpp
startup()
```

Better target:

```sh
qtcling --run examples/wiggly
```

or:

```sh
qtcling examples/wiggly/main.cpp
```

In these forms, qtcling should prepare generated files and a startup wrapper
behind the scenes.

## Generated work to hide

qtcling should automatically handle:

- `Q_OBJECT` in headers
  - generate `moc_<basename>.cpp`
- `Q_OBJECT` in source files
  - generate `<basename>.moc`
- `.ui`
  - generate `ui_<basename>.h`
- `.qrc`
  - generate `qrc_<basename>.cpp`
- `startup()`
  - generate a wrapper that includes selected source files and generated files
  - call `main(argc, argv)`
- translation files
  - run `lupdate` when source/UI files changed
  - run `lrelease` when `.ts` files changed
  - load resulting `.qm` files when the example expects translations

## Qt build-system feature checklist

Qt build systems do more than run `moc`, `uic`, and `rcc`. qtcling does not
need to support everything at once, but these items should stay visible when
designing `qtcling-project`.

### Core for qtcling

These are directly relevant to the current Widgets/C++ REPL workflow:

- Qt module selection
  - qmake: `QT += widgets network`, `QT -= gui`
  - CMake: `find_package(Qt6 COMPONENTS ...)`, `target_link_libraries(...)`
  - qtcling mapping: `QTCLING_MODULES`
- include paths
  - qmake: `INCLUDEPATH`
  - CMake: target include directories
  - qtcling mapping: Cling `-I`
- compile definitions and language standard
  - qmake: `DEFINES`, `CONFIG += c++20`, `CONFIG += c++23`
  - CMake: `target_compile_definitions`, `CMAKE_CXX_STANDARD`
  - qtcling mapping: Cling `-D`, `-std=...`
- `moc`
- `uic`
- `rcc`
- translations
  - `.ts` -> `.qm`
  - `lupdate`
  - `lrelease`
- startup wrapper generation
- app type differences
  - GUI app vs console app
  - macOS app bundle vs normal executable
  - Windows console subsystem vs GUI subsystem

### Resources

Qt resource handling has several forms:

- `.qrc` files compiled by `rcc`
- CMake `qt_add_resources(...)`
- CMake `qt_add_big_resources(...)`
- CMake `qt_add_binary_resources(...)`
- qmake `RESOURCES`
- generated resources from translations or QML

For early qtcling, `.qrc` -> `qrc_*.cpp` is enough. Later, `qtcling.toml` can
represent resource files without requiring a `.qrc` file.

### Plugins

Plugin-related features can be important even for small examples:

- `Q_PLUGIN_METADATA(...)`
  - handled by `moc`
  - may reference a JSON metadata file
- static plugin imports
  - `Q_IMPORT_PLUGIN(...)`
  - qmake: `QTPLUGIN`
  - CMake: `qt_import_plugins(...)`
- dynamic plugin deployment
  - platform plugins
  - image format plugins
  - SQL driver plugins
  - TLS/network plugins
  - style plugins

For qtcling, dynamic Qt plugins matter earlier than static plugins. The REPL may
need clear diagnostics for missing platform plugins or image format plugins.

### QML and Qt Quick

This is not part of the first Widgets-focused workflow, but it is a major Qt
build-system area:

- `.qml`
- `qmldir`
- `qt_add_qml_module(...)`
- QML type registration
- QML plugin generation
- QML resource embedding
- QML import scanning
- QML deployment
- optional QML cache / compilation behavior depending on Qt version and build

If qtcling grows beyond Widgets, QML should be treated as a separate phase.

### Deployment

Normal Qt builds also help with deployment:

- runtime library deployment
- Qt plugin deployment
- Qt translation deployment
- QML import deployment
- platform-specific tools and scripts
  - Windows deployment
  - macOS bundle deployment
  - Linux runtime/plugin layout

This is less important for REPL execution, but important for experimental
release packaging and for a future `qtcling-project package` command.

### Platform packaging metadata

Qt build systems often handle platform-specific metadata:

- macOS bundle settings
  - `Info.plist`
  - bundle identifier
  - app icon
- Windows resources
  - `.rc`
  - manifest
  - app icon
- Android metadata
  - package information
  - permissions
  - ABI selection

This is not needed for the first autogen implementation, but should be kept out
of the core REPL path.

### Specialized Qt code generators

These are lower priority, but they are Qt-specific generated-code features:

- D-Bus
  - `qdbusxml2cpp`
  - CMake `qt_add_dbus_adaptor(...)`
  - CMake `qt_add_dbus_interface(...)`
- Qt Remote Objects
  - `repc`
  - `.rep` files
  - generated `rep_*.h`
- Qt Shader Tools
  - `qsb`
  - CMake shader integration
- Qt Quick 3D
  - material shader generation

These should not block the first `qtcling-project`, but `qtcling --explain`
should eventually be able to report that such files are detected but unsupported.

## Later future scope

The first `qtcling-project` should stay focused on Widgets/C++ examples.

The following areas are important Qt build-system features, but they should be
treated as later future work:

### QML / Qt Quick support

Future work:

- detect `.qml` and `qmldir`
- understand `qt_add_qml_module(...)` enough to explain project structure
- add QML import paths
- handle QML resource embedding
- run QML import scanning when needed
- support QML deployment later

Reason to defer:

QML has its own module/import/deployment model and should not complicate the
first Widgets-focused autogen implementation.

### Plugin support beyond diagnostics

Future work:

- support static plugin imports
- understand `QTPLUGIN`
- understand CMake `qt_import_plugins(...)`
- help deploy dynamic plugins
- provide clear missing-plugin diagnostics

Reason to defer:

Dynamic plugins matter for runtime behavior, but full static-plugin and
deployment handling is not necessary for the first REPL workflow.

Important limitation:

Qt plugins should not be treated like `moc`, `uic`, or `rcc` outputs. A normal
dynamic Qt plugin is a shared library file (`.so`, `.dylib`, or `.dll`) with
metadata and entry points that `QPluginLoader` can inspect. Cling JIT code is
not such a plugin file.

Therefore, the following should be considered out of scope for qtcling-project:

- creating a normal Qt dynamic plugin purely from REPL input
- making JIT-generated code appear as a `QPluginLoader` plugin file
- adding static Qt plugins after the process has already started

Possible later directions:

- detect plugin projects and explain that they are unsupported
- load already-built plugin files with `QPluginLoader`
- invoke an external compiler/build step to build a plugin shared library, then
  load it
- for experimentation, avoid plugin loading and instantiate equivalent QObject
  implementations directly in the REPL

### Deployment and packaging

Future work:

- `qtcling-project package`
- Windows runtime / plugin deployment
- macOS bundle packaging
- Linux runtime / plugin layout
- Qt translation deployment
- QML import deployment

Reason to defer:

Packaging is about distributing an application. The first qtcling priority is
interactive execution without a build system.

### Platform metadata

Future work:

- macOS `Info.plist`
- bundle identifier
- app icon
- Windows `.rc`
- Windows manifest
- Android package metadata and permissions

Reason to defer:

These are application packaging details, not core REPL execution details.

### Specialized Qt generators

Future work:

- D-Bus
  - `qdbusxml2cpp`
  - `qt_add_dbus_adaptor(...)`
  - `qt_add_dbus_interface(...)`
- Qt Remote Objects
  - `repc`
  - `.rep`
  - `rep_*.h`
- Qt Shader Tools
  - `qsb`
  - shader baking
- Qt Quick 3D material generation

Reason to defer:

These are important for some Qt applications, but they are specialized. The
first implementation should only detect them and report that they are not yet
handled.

## Proposed command roles

### Internal prepare command

Create an internal command such as:

```sh
qtcling-prepare <project-or-directory>
```

This command may remain available for debugging, but normal users should not
need to run it directly.

It should:

1. Detect Qt tool executables.
2. Generate stale `moc`, `uic`, and `rcc` outputs.
3. Optionally update/release translations with `lupdate` and `lrelease`.
4. Generate a qtcling startup wrapper.
5. Print or write metadata needed by `qtcling`.

### User-facing commands

Possible user-facing forms:

```sh
qtcling --prepare .
qtcling --run .
qtcling --run main.cpp
qtcling --load startup.cpp
```

Long term, `qtcling --run .` is the cleanest target for small examples.

## Generated file location

Current examples often include generated files from the source directory. That
is easy but dirties the source tree.

Preferred future layout:

```text
.qtcling/
  autogen/
    moc_widget.cpp
    ui_form.h
    qrc_resources.cpp
  qtcling_startup.cpp
  manifest.json
```

The REPL should add `.qtcling/autogen` to the include path before loading the
generated startup wrapper.

For compatibility with current examples, an early implementation may still
generate next to sources. The `.qtcling/autogen` layout is the better long-term
target.

## Detection strategy

Start conservative.

### Phase 1: directory-local detection

For a specified directory:

- scan immediate files only
- run `moc` for `.h` / `.cpp` containing `Q_OBJECT`
- run `uic` for `.ui`
- run `rcc` for `.qrc`
- include `main.cpp` plus sibling `.cpp` files
- generate `startup()`

This covers many small examples.

### Phase 2: explicit file list

Support a small config or manifest when automatic source selection is ambiguous.

Example:

```toml
sources = ["main.cpp", "dialog.cpp", "wigglywidget.cpp"]
headers = ["dialog.h", "wigglywidget.h"]
forms = []
resources = []
main = "main.cpp"
```

This avoids guessing in larger examples.

### Phase 3: read project files

Read existing project metadata:

- `.pro`
- `CMakeLists.txt`
- later possibly `qtcling.toml`

Use `SOURCES`, `HEADERS`, `FORMS`, and `RESOURCES` to avoid including unwanted
files such as tests or alternate examples.

## Tool discovery

Qt tools should be found in this order:

1. explicit environment variables
   - `QTCLING_MOC`
   - `QTCLING_UIC`
   - `QTCLING_RCC`
   - `QTCLING_LUPDATE`
   - `QTCLING_LRELEASE`
2. `QTCLING_QT_ROOT/bin`
3. `PATH`
4. future `qtcling.toml`

`qtcling --show-config` should eventually show the resolved tool paths.

## Staleness

Generated files should only be regenerated when needed:

- output missing
- input newer than output
- qtcling autogen version changed
- relevant command path changed

Store metadata in:

```text
.qtcling/manifest.json
```

The first implementation can regenerate every time for simplicity, but the
final behavior should avoid unnecessary churn.

## Startup wrapper generation

The generated startup wrapper should:

- include generated `ui_*.h`, `qrc_*.cpp`, and `moc_*.cpp` when needed
- include selected `.cpp` files
- avoid including generated files twice
- define `startup()`
- call `main(argc, argv)` if a `main` function exists

Open questions:

- How to handle examples with multiple `main.cpp` files.
- How to detect source files that should not be included.
- How to handle examples that already provide `startup.cpp`.
- Whether to call `startup()` automatically after loading the wrapper.

## Extra ideas

### Priority additions

The most useful additions to keep visible are:

1. `qtcling --run .`
   - prepare generated files
   - generate/load startup wrapper
   - optionally call `startup()`
2. `qtcling --explain-autogen .`
   - show what would be generated
   - show which source files would be included
   - help debug wrong automatic detection
3. `qtcling.toml`
   - provide explicit file lists when automatic detection is ambiguous
   - avoid trying to fully emulate qmake or CMake at first
4. `qtcling-project`
   - integrate Qt helper tools in one command, similar in spirit to
     `pyside6-project`
   - hide `moc`, `uic`, `rcc`, `lupdate`, `lrelease`, and startup wrapper
     generation behind one workflow

### Autoload on `.L`

When the user runs:

```cpp
.L main.cpp
```

qtcling could prepare the containing directory first. This likely needs a Cling
MetaProcessor hook or a wrapper command, so it is not the first implementation
target.

### `qtcling --run .`

This is the best small-example workflow:

1. prepare directory
2. start Cling
3. load generated startup wrapper
4. optionally call `startup()`

### `qtcling.toml`

For examples that cannot be guessed safely, use a small explicit file:

```toml
[project]
sources = ["main.cpp", "dialog.cpp", "wigglywidget.cpp"]
headers = ["dialog.h", "wigglywidget.h"]
forms = []
resources = []
translations = ["colorwidget_ja.ts"]
startup = "auto"
```

This is simpler than trying to fully emulate qmake or CMake.

### `qtcling-project`

PySide6 has a useful precedent in project-level helper commands that hide
several Qt tools behind one workflow. qtcling should consider a similar command:

```sh
qtcling-project prepare .
qtcling-project run .
qtcling-project clean .
qtcling-project explain .
```

This command would be the user-visible project helper, while lower-level pieces
such as `qtcling-prepare` can stay internal.

Qt Creator's Python project support is a particularly relevant model:

- Python projects can be opened through `pyproject.toml`.
- Qt for Python project wizards generate `pyproject.toml`.
- The project file lists source files and configuration.
- Generated files are not meant to be listed as project sources.
- `pyside6-project` uses the project file for build/run/deploy/lupdate/clean
  style operations.

qtcling can mirror this with:

```text
pyproject.toml     -> qtcling.toml
pyside6-project    -> qtcling-project
Python run config  -> qtcling run config / qtcling-server session
```

This makes `qtcling.toml` a good bridge between command-line qtcling workflows
and a future Qt Creator plugin. The plugin can show `Sources`, `Headers`,
`Forms`, `Resources`, and `Translations` from the TOML file in a project view.

Possible responsibilities:

- discover Qt tools
- run `moc`
- run `uic`
- run `rcc`
- run `lupdate`
- run `lrelease`
- generate `.qtcling/autogen/qtcling_startup.cpp`
- write `.qtcling/manifest.json`
- print a clear explanation of generated files

### Translation automation

Translation support should follow the same principle: users should not need to
manually run Qt translation tools while experimenting.

Possible behavior:

- If `.ts` files exist, run `lrelease` to produce `.qm` files when stale.
- If `QTCLING_UPDATE_TRANSLATIONS=1` or `qtcling-project update-translations`
  is used, run `lupdate` first.
- Include `.ui` files in `lupdate` inputs.
- Detect translation resource `.qrc` files and regenerate `qrc_*.cpp` after
  `.qm` files change.

`lupdate` can modify `.ts` files, so it should probably not run by default on
every `qtcling --run .`. A safer default is:

- default: run stale `lrelease`
- opt-in: run `lupdate`

Candidate commands:

```sh
qtcling-project update-translations .
qtcling-project release-translations .
qtcling --run . --release-translations
```

### Dry-run / explain mode

Add:

```sh
qtcling --prepare --dry-run .
qtcling --explain-autogen .
```

This would show what qtcling would generate without changing files. It is useful
when automatic detection is wrong.

### Cleanup

Add:

```sh
qtcling --clean .
```

This removes `.qtcling/autogen` but does not touch user sources.

### Integration with Qt Creator plugin plan

The future `qtcling-server` backend can reuse the same autogen engine. Qt
Creator plugin can provide project file lists and Qt Kit information, while the
backend performs prepare/load/eval.
