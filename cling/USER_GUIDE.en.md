---
genpdf:
  format: book
  title: qtcling 1.2.0 User Guide
  author: SRA, Inc.
  font_size: 12pt
  page_numbers: true
  copyright: Copyright (c) 2026 SRA, Inc. All rights reserved.
  table:
    header_background: "#e8f1ff"
    header_color: "#1f2937"
    border_color: "#cbd5e1"
    border_width: "1px"
    stripe: true
    stripe_background: "#f8fafc"
    cell_padding: "0.45em 0.65em"
    font_size: "0.95em"
    compact: false
    align: left
    header_align: center
    cell_align: left
---

# qtcling 1.2.0 User Guide

## What Is qtcling?

qtcling is an environment for using Qt interactively from the Cling C++ REPL.

It lets you create and display Qt objects and widgets in the REPL and check signal/slot and property behavior without waiting for a normal build.

## Current Status

Version 1.2.0 is a source code release.

The user-facing scope of 1.2.0 is terminal-based interactive `qtcling` on macOS, Linux, and native Windows 11 ARM64.

Version 1.1.0 introduced the macOS/Linux release. Version 1.2.0 officially adds native Windows 11 ARM64 support.

On macOS, QtCore, QtGui, QtWidgets, and event callbacks have been verified in terminal-based `qtcling`.

On Linux, `--show-config`, `--check`, and Qt application initialization have been verified in terminal-based `qtcling`.

On native Windows 11 ARM64, REPL editing, history, Qt events, widgets, signals/slots, and Qt message output have been verified. See [`docs/windows-native-arm64.md`](docs/windows-native-arm64.md) for details.

`qtcling-gui`, `qtcling-project`, `qtcling.toml`, and the Qt Creator plugin concept are outside the user-facing scope of 1.2.0.

## macOS Quick Start

The following basic environment variables have been verified:

```sh
export QTCLING_QT_ROOT=/usr/local/qt/Qt/6.11.1/macos
export QTCLING_CLING_ROOT="$PWD/build"
export QTCLING_STARTUP_FILE="$PWD/src/qtgui.cpp"
```

If you have not yet installed Cling with the hooks under `/usr/local/qtcling`, specify `QTCLING_CLING_ROOT="$PWD/build"`.

Start the terminal version:

```sh
qtcling
```

After startup, verify that `qApp` is available:

```cpp
qApp
```

Create and display a widget:

```cpp
#include <QPushButton>
auto button = new QPushButton("OK?");
button->show()
```

## Using the Source Code Release ZIP

The distributed ZIP is a source code release. It does not include `build/`, `cling/`, `llvm-project/`, or an installed copy of Qt.

Extract it:

```sh
unzip qtcling-1.2.0-source.zip
cd qtcling-1.2.0
```

## Installation Procedure

In addition to Qt, `qtcling` requires Cling with the interactive patches applied.

### Dependencies

Set the default Qt installation path in [`bin/qtcling-defaults.sh`](bin/qtcling-defaults.sh). For a persistent setting, change `QTCLING_DEFAULT_QT_ROOT` in that file to the full Qt kit path before building. To override it only in the current shell, export the environment variable.

```sh
export QTCLING_DEFAULT_QT_ROOT=/usr/local/qt/Qt/6.11.1/macos
./build.sh
```

`bin/qtcling-defaults.sh` is read by both the build and launcher wrappers and is copied to the installation destination. `QTCLING_QT_ROOT` specifies a complete kit path for an individual launch and takes precedence over `QTCLING_DEFAULT_QT_ROOT`.

On Linux, LLVM's libedit LineEditor support is required to process Qt events while the REPL waits for input. On Ubuntu, install `libedit-dev` before building.

```sh
sudo apt install libedit-dev
```

Without `libedit-dev`, the build may continue, but Qt events may not be processed while the REPL waits for input, causing button clicks and similar events to respond together only after Enter is pressed. `build.sh` warns if it detects `LLVM_ENABLE_LIBEDIT=OFF`.

On macOS, libedit from the Xcode SDK is used.

`libzstd` is optional and is not required for normal interactive qtcling use. LLVM/CMake may detect zstd in the environment, but zstd support does not affect ordinary Qt REPL use.

The distributed `build.sh` performs these steps:

1. If `cling/` is missing, clone `root-project/cling` and pin revision `af630d98`.
2. If `llvm-project/` is missing, clone `root-project/llvm-project` and pin revision `7c49650f1446`.
3. Validate revisions and tracked changes in existing checkouts.
4. Validate and apply the required patches in `patch/qtcling-interactive/`.
5. Build Cling in Release mode under `build/`.

If a required patch cannot be applied and is not already applied, `build.sh` stops at that point.

Example:

```sh
./build.sh
```

`build.sh` builds only; it does not install.

### Installing Before Use

After `build.sh` completes, run this from the repository root:

```sh
sudo ./install.sh
```

`install.sh` installs Cling, the qtcling launcher, platform wrappers, default path settings, startup file, and `QTCLING_VERSION` under the prefix specified by CMake. The default is `/usr/local/qtcling`.

In this configuration, the macOS wrapper uses `/usr/local/qtcling/bin/cling` by default, so `QTCLING_CLING_ROOT` is normally unnecessary.

On macOS, specify the Qt root and check the installation:

```sh
export QTCLING_QT_ROOT=/usr/local/qt/Qt/6.11.1/macos
bin/qtcling --check
bin/qtcling --show-config
bin/qtcling
```

### Using the Build Tree Without Installing

If you do not install under `/usr/local/qtcling`, specify the build tree with `QTCLING_CLING_ROOT`.

```sh
export QTCLING_QT_ROOT=/usr/local/qt/Qt/6.11.1/macos
export QTCLING_CLING_ROOT="$PWD/build"
bin/qtcling --check
bin/qtcling --show-config
bin/qtcling
```

`$QTCLING_CLING_ROOT/bin/cling` will be used.

### Cloning, Patching, and Building Manually

To proceed manually without `build.sh`, run the following in order:

```sh
git clone https://github.com/root-project/cling.git
git -C cling checkout --detach af630d98
git clone https://github.com/root-project/llvm-project.git
git -C llvm-project checkout --detach 7c49650f1446
git apply patch/qtcling-interactive/0001-cling-add-periodic-callback-api.patch
git apply patch/qtcling-interactive/0002-llvm-lineeditor-periodic-callback.patch
git apply patch/qtcling-interactive/0003-cling-load-qtcling-startup-file.patch
git apply patch/qtcling-interactive/0006-warn-when-libedit-is-disabled.patch
mkdir -p build
cd build
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local/qtcling \
  -DLLVM_ENABLE_PROJECTS=clang \
  -DLLVM_EXTERNAL_PROJECTS=cling \
  -DLLVM_EXTERNAL_CLING_SOURCE_DIR=../cling \
  -DLLVM_BUILD_TOOLS=OFF \
  -DLLVM_TARGETS_TO_BUILD=host \
  ../llvm-project/llvm
cmake --build . -j 2
```

To install:

```sh
cd ..
sudo ./install.sh
```

`install.sh` places Cling, the qtcling launcher, platform wrappers, default path settings, startup file, and `QTCLING_VERSION` under the installation prefix configured by CMake at build time. The default prefix is `/usr/local/qtcling`.

To use it without installing:

```sh
cd ..
export QTCLING_CLING_ROOT="$PWD/build"
```

### startup file

The source release wrapper automatically treats the extracted directory as `QTCLING_DEV_ROOT`.

Normally, `src/qtgui.cpp` in that directory is used without explicitly setting `QTCLING_STARTUP_FILE`.

Set it only when selecting a different startup file:

```sh
export QTCLING_STARTUP_FILE="$PWD/src/qtgui.cpp"
```

## Verifying Startup

On macOS, specify the Qt root first:

```sh
export QTCLING_QT_ROOT=/usr/local/qt/Qt/6.11.1/macos
```

To verify the installed Cling and launcher, run them directly from the installation destination:

```sh
/usr/local/qtcling/bin/qtcling --check
/usr/local/qtcling/bin/qtcling --show-config
/usr/local/qtcling/bin/qtcling
```

The source tree's `bin/qtcling` prefers the build tree over the installation when `build/bin/cling` exists. To use the build tree without installing, or to make the choice explicit, set `QTCLING_CLING_ROOT` first.

```sh
export QTCLING_CLING_ROOT="$PWD/build"
bin/qtcling --check
```

Add the launcher directory to PATH if needed:

```sh
export PATH="$PWD/bin:$PATH"
```

## Native Windows 11 ARM64

Native Windows 11 ARM64 is officially supported in 1.2.0. On physical hardware, startup through `bin/qtcling.cmd`, libedit line editing and history, `qApp`, Qt widgets, Qt events while waiting for input, `QObject::connect`, and `qDebug()` console output have been verified.

### Prerequisites

Prepare the following native ARM64 environment. Avoid mixing it with x64 versions.

- Windows 11 ARM64
- Visual Studio 2022 Build Tools
  - Desktop development with C++
  - MSVC v143 C++ ARM64 build tools
  - Windows 11 SDK (verified with 10.0.26100.0)
- CMake 3.21 or later
- Git for Windows (including Git Bash)
- Python 3.13 ARM64
- Qt 6.11.1 `msvc2022_arm64` kit

Verified installation locations:

```text
Python C:\Users\<user>\AppData\Local\Programs\Python\Python313-arm64\python.exe
Qt     C:\Qt\6.11.1\msvc2022_arm64
```

Check command availability in PowerShell:

```powershell
git --version
cmake --version
& "$env:LOCALAPPDATA\Programs\Python\Python313-arm64\python.exe" --version
Test-Path C:\Qt\6.11.1\msvc2022_arm64\include\QtCore
```

### Preparing the qtcling Source

Extract the 1.2.0 source archive to obtain the qtcling source root, including the native Windows support files. You only need to obtain qtcling's own source yourself. In Git Bash, change to the root of the downloaded or extracted qtcling development source. The directory name is arbitrary; this working environment uses `C:\Users\sugita\cling`. After changing directories, verify the following files exist:

```sh
cd /c/Users/sugita/cling

test -f build-win.sh
test -f build-libedit-win.ps1
test -f patch/libedit-windows-arm64.patch
test -f windows/CMakeLists.txt
test -f src/qtguiwin.cpp
test -f src/qtguiwin-startup.cpp

```

You do not need to obtain Cling, LLVM, or portable libedit manually. The subsequent `build-win.sh` and `build-libedit-win.ps1` scripts download them and pin their revisions automatically.

```text
Cling            af630d98
LLVM             7c49650f1446
portable libedit f3a98319002d0eca05e2a370a68b28a2afa6a822
```

If an existing checkout is at a different revision and has tracked changes, the script stops to protect those changes.

### Building and Installing libedit First

Before configuring Cling, build portable libedit in PowerShell and install it under `libedit-win-arm64/`.

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\build-libedit-win.ps1
```

This script clones libedit, checks out the pinned revision, applies `patch/libedit-windows-arm64.patch`, and builds and installs an ARM64 Release version. After it finishes, check the required files:

```powershell
Test-Path .\libedit-win-arm64\lib\edit.lib
Test-Path .\libedit-win-arm64\share\libedit\termcap
```

Verify that both return `True` before proceeding to Cling/LLVM.

### Building and Installing Cling/LLVM

Run the following from Git Bash:

```sh
./build-win.sh
```

For safety, `build-win.sh` also invokes `build-libedit-win.ps1` before configuring Cling. If the preceding step is complete, libedit is built incrementally. The full script runs in this order:

1. Clone Cling and LLVM as needed and verify their pinned revisions.
2. Check the ARM64 Python installation.
3. Build portable libedit and install it under `libedit-win-arm64/`.
4. Validate and apply common patches `0001`, `0002`, `0003`, and `0006`, and Windows patches `0007`, `0008`, and `0009`.
5. Configure Cling/LLVM using the installed `libedit-win-arm64/`.
6. Build Cling/LLVM with Visual Studio 2022 ARM64 Release.
7. Install the Release build of Cling under `qtcling-win-arm64/`.

Cling/LLVM uses `-j 2` to suit the verified environment with 12 GB of memory. The build may take more than 30 minutes. If interrupted, rerun the same command to resume incrementally.

Check the Cling installation:

```powershell
Test-Path .\qtcling-win-arm64\bin\cling.exe
```

### Building and Placing the Qt Callback DLL

Build the ARM64 DLL in PowerShell:

```powershell
cmake -S .\windows -B .\build-qtcling-win-runtime `
  -G "Visual Studio 17 2022" -A ARM64 `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\msvc2022_arm64

cmake --build .\build-qtcling-win-runtime --config Release -j 2

Copy-Item `
  .\build-qtcling-win-runtime\bin\Release\qtcling-win.dll `
  .\bin\qtcling-win.dll -Force
```

Verify its location:

```powershell
Test-Path .\bin\qtcling-win.dll
```

`qtcling-win.dll` is a native ARM64 DLL that processes Qt events while the REPL waits for input and forwards Qt messages to the console. Even if you move the source tree, keep the DLL in the same `bin` directory as `bin/qtcling.cmd`.

### Installing on Windows

After building Cling/LLVM and libedit, run the following in PowerShell:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\install-win.ps1
```

The default destination is `%LOCALAPPDATA%\qtcling`. `install-win.ps1` builds the Qt callback DLL as ARM64 Release with `-j 2` and installs the following files:

```text
bin\cling.exe
bin\qtcling.cmd
bin\qtcling-win.dll
share\libedit\termcap
src\qtguiwin-startup.cpp
QTCLING_VERSION
```

To change the installation destination, specify a Windows-style path:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\install-win.ps1 `
  -InstallDir C:\Tools\qtcling
```

If Qt is installed outside the standard location, also specify `-QtRoot`:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\install-win.ps1 `
  -QtRoot D:\Qt\6.11.1\msvc2022_arm64
```

To launch from the default installation destination:

```powershell
& "$env:LOCALAPPDATA\qtcling\bin\qtcling.cmd"
```

### Starting and Verifying the Installation

Launch from Git Bash:

```sh
bin/qtcling.cmd
```

If Qt is installed outside the standard location, specify its Windows-style path before launching:

```sh
export QTCLING_QT_ROOT='D:\Qt\6.11.1\msvc2022_arm64'
bin/qtcling.cmd
```

A successful launch displays the `[cling]$` prompt. First check:

```cpp
qApp
```

If `(QApplication *) 0x...` appears, the startup file and Qt callback DLL initialized successfully.

Example checks in the REPL:

```cpp
qApp
#include <QPushButton>
const auto b = new QPushButton("OK?");
QObject::connect(b, &QPushButton::clicked,
                 []() { qDebug() << "Clicked"; });
b->show();
```

Click the button and confirm that `Clicked` appears in the console. Verify that the up/down keys recall history and the left/right keys edit the input line. Use `.q` to exit.

### Output Locations and Rebuilding

```text
libedit/                    portable libedit source
build-libedit-win/          libedit build tree
libedit-win-arm64/          libedit install tree
cling/                      Cling source
llvm-project/               LLVM/Clang source
build-win-arm64/            Cling/LLVM build tree
qtcling-win-arm64/          Cling install tree
build-qtcling-win-runtime/  Qt callback DLL build tree
bin/qtcling-win.dll         Qt callback DLL loaded at startup
```

For a normal rebuild, rerun the following in Git Bash:

```sh
./build-win.sh
```

If only the Qt callback DLL changed, rerun just `cmake --build` and `Copy-Item` from "Building and Placing the Qt Callback DLL."

### Common Problems

- `ARM64 Python not found`: Check the Python 3.13 ARM64 installation. The current `build-win.sh` uses `%LOCALAPPDATA%\Programs\Python\Python313-arm64\python.exe`.
- `Qt ARM64 headers not found`: Point `QTCLING_QT_ROOT` to the `msvc2022_arm64` kit.
- `cling.exe not found`: Check that `./build-win.sh` completed its installation stage.
- Unable to load `qtcling-win.dll`: Copy the DLL to `bin/` and verify that Qt's `bin` is added to the launcher's `PATH`.
- `.\version: error: invalid suffix '.0' on floating constant`: An old `VERSION` file remains in the source root. Remove it because it conflicts with the standard C++ `<version>` header on Windows; use `QTCLING_VERSION` for the release version.
- Access violation immediately after the logo: Apply the `getline()` fix in `patch/libedit-windows-arm64.patch`, then relink libedit and Cling.
- Qt widgets appear but do not respond: Apply the latest `0002-llvm-lineeditor-periodic-callback.patch`, including the Windows LineEditor callback, and rebuild.

See [`docs/windows-native-arm64.md`](docs/windows-native-arm64.md) for build configuration, pinned revisions, portable libedit fixes, the Windows-specific Qt callback DLL, and troubleshooting.

## Distribution Contents

The public source archive includes these main files:

- `build.sh` / `install.sh`
- `build-win.sh` / `build-libedit-win.ps1` / `install-win.ps1`
- `QTCLING_VERSION`
- `bin/qtcling`
- `bin/qtcling.cmd`
- `bin/iqtcling.macos` / `bin/cqtcling.macos`
- `bin/iqtcling.linux` / `bin/cqtcling.linux`
- `bin/qtcling-defaults.sh`
- `bin/run_moc`
- `bin/run_uic`
- `bin/run_rcc`
- `bin/run_all`
- `patch/qtcling-interactive.patch`
- `patch/qtcling-interactive/`
- `patch/libedit-windows-arm64.patch`
- `examples/`
- `src/`
- `windows/`

## Applying the Interactive Patches

Interactive event loop support in `qtcling` requires patches to `cling/` and `llvm-project/`. Normally `build.sh` applies them automatically, so the following is needed only for manual builds.

With `cling/` and `llvm-project/` in the top-level checkout, apply the required patches included in the distribution:

```sh
git apply patch/qtcling-interactive/0001-cling-add-periodic-callback-api.patch
git apply patch/qtcling-interactive/0002-llvm-lineeditor-periodic-callback.patch
git apply patch/qtcling-interactive/0003-cling-load-qtcling-startup-file.patch
git apply patch/qtcling-interactive/0006-warn-when-libedit-is-disabled.patch
```

The combined `patch/qtcling-interactive.patch` simply concatenates these four required patches.

To inspect them individually, see the patches under `patch/qtcling-interactive/`:

- `0001-cling-add-periodic-callback-api.patch`
- `0002-llvm-lineeditor-periodic-callback.patch`
- `0003-cling-load-qtcling-startup-file.patch`
- `0006-warn-when-libedit-is-disabled.patch`

`0004-qtcling-wrapper-macos-linux-fixes.patch` and `0005-linux-wrapper-diagnostics.patch` are reference patches for the top-level wrappers. Current checkouts already include those wrapper changes in Git, so they are normally not applied.

## Terminal qtcling

`qtcling` uses the Cling REPL in a terminal.

```sh
qtcling
```

The macOS interactive version loads `src/qtgui.cpp` at startup through `QTCLING_STARTUP_FILE`. This prepares `QApplication` without requiring a manual `.L qtgui.cpp`.

Example of loading an example:

```cpp
.I examples
.L button.cpp
```

`examples/button.cpp` prints `OK?` when the button is clicked. This verifies that Qt events are processed even while the REPL waits for input.

## GUI Version: qtcling-gui

`qtcling-gui` is a prototype with REPL-style input and output fields inside a Qt GUI application. It is not included in the public 1.2.0 source archive and is available only in a development checkout.

```sh
qtcling-gui
```

It runs Cling within Qt's `QApplication::exec()` without depending on the terminal `LineEditor` or libedit.

Example:

```cpp
qApp
#include <QPushButton>
auto button = new QPushButton("OK?");
button->show()
```

Currently, only single-line input is supported. History, completion, multiline input, and similar features are not implemented.

## Environment Variables

### `QTCLING_QT_ROOT`

Specifies the Qt installation root.

```sh
export QTCLING_QT_ROOT=/usr/local/qt/Qt/6.11.1/macos
```

On macOS, include and framework paths are constructed from `lib/*.framework` under this root.

### `QTCLING_CLING_ROOT`

Specifies the root of the Cling installation to use.

```sh
export QTCLING_CLING_ROOT="$PWD/build"
```

`$QTCLING_CLING_ROOT/bin/cling` will be used.

### `QTCLING_STARTUP_FILE`

Specifies the startup file loaded when qtcling starts.

```sh
export QTCLING_STARTUP_FILE="$PWD/src/qtgui.cpp"
```

This file prepares `QApplication` and configures the Qt event processing callback.

### `QTCLING_MODULES`

Specifies the Qt modules to use.

```sh
export QTCLING_MODULES="Core Gui Widgets Network"
```

The default is `Core Gui Widgets`.

Example:

```sh
QTCLING_MODULES="Core Gui Widgets Network" qtcling
```

In the REPL:

```cpp
#include <QTcpSocket>
auto socket = new QTcpSocket;
socket
```

### `QTCLING_RESOURCE_DIR`

Use this to explicitly specify the Clang resource directory for `qtcling-gui`.

```sh
export QTCLING_RESOURCE_DIR="$PWD/build/lib/clang/20"
```

Normally, the build-time default is used.

## Checking Configuration

The macOS/Linux interactive versions provide configuration-checking options:

```sh
qtcling --version
qtcling --show-config
qtcling --check
```

`--version` displays the qtcling release version from `QTCLING_VERSION` in the distribution root.

`--show-config` displays the Qt root, modules, framework or library paths, Cling executable, startup file, and Cling arguments.

`--check` verifies that the Qt root, Qt libraries, Cling executable, and startup file exist.

## Adding Qt Modules

Set `QTCLING_MODULES` to use additional modules:

```sh
QTCLING_MODULES="Core Gui Widgets Network" qtcling
```

On macOS, the specified modules determine include paths and framework loading settings. On Linux, they determine include paths, `.so` preloads, and `-lQt6...` linker arguments.

## qtcling-project

`qtcling-project` is a minimal prototype of a future project helper. It is not included in the public 1.2.0 source archive and is available only in a development checkout.

Only `explain` is currently implemented. It does not yet generate files or launch applications automatically at project level.

```sh
qtcling-project explain examples/wiggly
```

The output includes:

- Project directory
- Whether `qtcling.toml` exists
- Whether a `.pro` file exists
- Sources
- Headers
- Forms
- Resources
- Translations
- Qt modules
- Include directories
- Defines
- Autogen outputs
- Generated startup includes
- Warnings
- Unsupported features

Example:

```sh
qtcling-project explain examples/widgets/tutorials/notepad
```

Plugin projects are outside the initial scope. If detected, they appear under `Unsupported`.

```sh
qtcling-project explain examples/cpp-advanced/solutions/sif
```

## Creating Qt Generated Files

Ordinary Qt code depends on files produced by `moc`, `uic`, and `rcc`.

For example, a class with `Q_OBJECT` needs `moc` output. Code using `.ui` needs `ui_*.h`, and code using `.qrc` needs `qrc_*.cpp`. Without generating these files, most Qt examples cannot run unchanged.

qtcling provides helpers for manual execution:

```sh
run_moc [files...]
run_uic [files...]
run_rcc [files...]
run_all
```

`run_moc` runs `moc` only on the specified C++ headers or sources that contain `Q_OBJECT`.

| Input | Output |
|---|---|
| `widget.h` | `moc_widget.cpp` |
| `widget.cpp` | `widget.moc` |

`run_uic` generates `ui_<basename>.h` from `.ui` files.

| Input | Output |
|---|---|
| `mainwindow.ui` | `ui_mainwindow.h` |

`run_rcc` generates `qrc_<basename>.cpp` from `.qrc` files.

| Input | Output |
|---|---|
| `resources.qrc` | `qrc_resources.cpp` |

With no arguments, each helper processes matching files in the current directory.

Example:

```sh
cd examples/wiggly
run_all
```

`run_all` executes `run_moc`, `run_rcc`, and `run_uic` in that order in the current directory.

```sh
run_all
```

If `moc`, `uic`, or `rcc` is not on PATH, specify its location to the corresponding helper:

```sh
run_moc --moc /path/to/moc widget.h
run_uic --uic /path/to/uic mainwindow.ui
run_rcc --rcc /path/to/rcc resources.qrc
```

### Translation Files

Projects using `TRANSLATIONS` or `.ts` files may need `.qm` files, just as in a normal Qt build.

Currently, `qtcling-project explain` lists translations, but qtcling does not run `lrelease`, `lupdate`, or `lconvert`.

- `lrelease`: Generates `.qm` from `.ts`. A future `qtcling-project prepare` is planned to regenerate stale `.qm` files.
- `lupdate`: Updates `.ts` files. Because it modifies the user's translation files, it will not run automatically by default.
- `lconvert`: Converts translation file formats and performs related operations. It is not currently supported.

For projects using translations, run Qt's tools directly as needed:

```sh
lrelease *.ts
```

If a project includes `.qm` files in a `.qrc`, update the `.qm` files before generating resources:

```sh
lrelease *.ts
run_rcc *.qrc
```

Alternatively, run `run_all` as usual after updating the `.qm` files:

```sh
lrelease *.ts
run_all
```

## Running Examples

The current basic procedure is to generate the required files first, add the target directory to the include path, load `startup.cpp`, and call `startup()`.

Example:

```sh
cd examples/wiggly
run_all
cd ../..
```

```sh
qtcling
```

In the REPL:

```cpp
.I examples/wiggly
.L examples/wiggly/startup.cpp
startup()
```

Future `qtcling-project prepare` and `qtcling --run .` commands are planned to hide the details of generating `moc`, `uic`, `rcc`, and `startup()`.

## Current Limitations

- Version 1.2.0 is a source code release.
- Its user-facing scope is terminal-based interactive `qtcling` on macOS, Linux, and native Windows 11 ARM64.
- `qtcling-gui` is a prototype outside the user-facing scope of 1.2.0.
- `qtcling-project` provides only `explain` and is outside the user-facing scope of 1.2.0.
- Helpers can run `moc`, `uic`, and `rcc` manually, but project-level automatic execution is not implemented.
- `lrelease`, `lupdate`, and `lconvert` integration is not implemented.
- `qtcling.toml` has only initial read support.
- QML, plugins, deployment, D-Bus, Remote Objects, and ShaderTools are outside the initial scope.
- A plugin project cannot be treated as a normal Qt plugin through the REPL alone.

## Troubleshooting

### `qApp` Is Undefined

Check that you are using Cling with the hooks:

```sh
export QTCLING_CLING_ROOT="$PWD/build"
qtcling
```

Also check that `QTCLING_STARTUP_FILE` is correct:

```sh
qtcling --check
```

### `resource directory lib/clang/20 not found`

`qtcling-gui` cannot find the Clang resource directory.

```sh
export QTCLING_RESOURCE_DIR="$PWD/build/lib/clang/20"
qtcling-gui
```

### A Qt Module Header Cannot Be Found

Add the module to `QTCLING_MODULES`:

```sh
QTCLING_MODULES="Core Gui Widgets Network" qtcling
```

### A Plugin Example Does Not Work

Plugins are outside the initial scope. If `qtcling-project explain` marks an example as unsupported, it is not handled by the normal autogen workflow.