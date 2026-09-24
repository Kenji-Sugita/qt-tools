# qeyes

`qeyes` is a small Qt Widgets application that displays two eyes on the desktop,
following the direction of the mouse pointer.

## Features

- Frameless window with a transparent background
- Always on top by default
- Irises and pupils follow the mouse pointer
- Move by dragging with the left mouse button
- Exit with a left double-click
- Right-click to open the appearance settings dialog

## Building

```text
$ cmake -S . -B build
$ cmake --build build
```

## Running

```text
$ ./build/qeyes
```

## Basic Controls

- Left drag: Move the window
- Left double-click: Exit
- Right-click: Open the appearance settings dialog

## Related Documents

- Documentation index: `docs/README.md`
- User Guide 1.0: `docs/guides/USER_GUIDE-1.0.md`
- User Guide 1.1: `docs/guides/USER_GUIDE-1.1.md`
- User Guide 1.2: `docs/guides/USER_GUIDE-1.2.md`
- Release Notes 1.0: `docs/releases/ReleaseNote-1.0.md`
- Release Notes 1.1: `docs/releases/ReleaseNote-1.1.md`
- Release Notes 1.2: `docs/releases/ReleaseNote-1.2.md`
- Release history: `docs/history/RELEASE_HISTORY.md`
