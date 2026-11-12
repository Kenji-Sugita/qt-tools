# qtcling-gui prototype

Qt-native GUI REPL host prototype for qtcling.

This mode is intentionally separate from the terminal `qtcling` wrapper.
It uses `QApplication::exec()` as the main event loop and evaluates Cling input
on the Qt main thread, so it does not depend on LineEditor/libedit.

## Build

```sh
cmake -S qtcling-gui -B build-qtcling-gui \
  -DCling_DIR="$PWD/build/tools/cling/lib/cmake/cling" \
  -DQt6_DIR=/usr/local/qt/Qt/6.11.1/macos/lib/cmake/Qt6

cmake --build build-qtcling-gui
```

## Run

```sh
QTCLING_QT_ROOT=/usr/local/qt/Qt/6.11.1/macos \
QTCLING_MODULES="Core Gui Widgets" \
bin/qtcling-gui
```

The prototype opens a small window with an output panel and one-line input.
Press Enter or the Run button to evaluate the line through Cling.

## Notes

- This is a prototype, not a replacement for terminal `qtcling`.
- Evaluation runs on the Qt main thread.
- `QTCLING_STARTUP_FILE` is honored when set.
- `QTCLING_CLING_ROOT` and `QTCLING_RESOURCE_DIR` can override the embedded
  defaults when the Cling build tree is not at the path used at build time.
- `cling_set_periodic_callback` and `cling_clear_periodic_callback` are no-op
  in this mode because Qt owns the event loop.
