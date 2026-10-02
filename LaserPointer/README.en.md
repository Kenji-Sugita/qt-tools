# LaserPointer

[日本語](README.md) | English

LaserPointer is a Qt Widgets application that overlays a laser spot on the screen
to highlight areas during presentations, seminars, and screen sharing.

Drag the spot to any position, or enable mouse-pointer tracking.

## Main Features

- Transparent, frameless, always-on-top pointer
- Adjustable size, color, opacity, and shape (Glow / Ring / Cross / Star)
- Blinking, rainbow colors, white click ripples, and temporary enlargement with Space
- Cursor tracking, smooth tracking, and screen-boundary constraints
- Trails (Glow / Dots), automatic fading, and a hold-H-to-show mode
- Presets and settings persistence through `QSettings`
- Menu bar controls on macOS and system tray controls on Windows

Menus and help appear in Japanese when the system language is Japanese, and in English otherwise.

## Build

Requirements: CMake 3.16 or later, a C++17 compiler, and Qt 6 Widgets.
Use a Qt installation that provides `qt_standard_project_setup()`.
The current project version in `CMakeLists.txt` is `1.0.0`.

Run from this `LaserPointer/` directory:

```sh
cmake -S . -B build
cmake --build build --config Release
```

If Qt cannot be found, specify its installation prefix during configuration:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/platform
```

On macOS, the default configuration builds an arm64 / x86_64 universal binary
unless architectures are explicitly specified.

## Run

macOS:

```sh
open build/LaserPointer.app
```

Linux:

```sh
./build/LaserPointer
```

Windows (Release build with a multi-configuration generator):

```powershell
.\build\Release\LaserPointer.exe
```

Output paths depend on the CMake generator. Examples include `build/LaserPointer.exe`
for a single-configuration build and `build/Release/LaserPointer.app` for a
multi-configuration macOS build. On macOS / Windows, the status icon also lets
you show or hide the pointer. On Linux, use the pointer's right-click menu.

## Basic Controls

| Operation | Action |
|---|---|
| Left drag | Move the spot when cursor tracking is off |
| Left click | Show a white ripple |
| Mouse wheel / `+` / `-` | Resize |
| Right click | Open the settings menu |
| `F` | Toggle cursor tracking |
| `T` | Toggle trails |
| `Space` | Enlarge while held |
| `R` | Reset all settings |
| `Esc` / `Q` | Quit |

On first launch, the pointer is red, size 96, and 100% opaque, with trails and
cursor tracking disabled. Settings are restored on subsequent launches; position
is not saved, and the pointer starts at the center of the screen.

## Related Documents

- [User Guide](USER_GUIDE.en.md): Settings, all shortcuts, and troubleshooting
- [User Guide PDF (Japanese)](USER_GUIDE.pdf)
- [Cheat Sheet PNG (Japanese)](laserpointer-cheatsheet.png) / [SVG](laserpointer-cheatsheet.svg)

![LaserPointer controls cheat sheet (Japanese)](laserpointer-cheatsheet.png)
