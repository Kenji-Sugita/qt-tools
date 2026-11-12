# Anker Layout

[日本語](README.md)

Anker Layout is a `QLayout` implementation that provides form-style layouts similar to Motif's `XmForm` for Qt Widgets.
It positions widgets by attaching their left, right, top, and bottom edges to the form, other widgets, proportional positions, or their own initial positions.
No Motif library is required.

## Features

- Attach widgets to form edges and stretch them between two attached edges
- Specify spacing between widgets and align their edges
- Position widgets at fractions of the form's width or height
- Use initial positions, with optional proportional movement as the form resizes
- Update constraints with `setConstraints()` and retrieve them with `constraintsFor()`

Pass `FormConstraints` to `Anker::FormLayout::addWidget()`. For example, this places an editor to the right of a label and stretches it to the right edge of the form:

```cpp
layout->addWidget(editor, {
    .left = Anker::attachWidget(label, 8),
    .right = Anker::attachForm(),
    .top = Anker::attachOppositeWidget(label),
});
```

The library consists of three headers in `include/` and two implementation files in `src/`.
Include `AnkerFormLayout.h` in code that uses it.

## Building and Testing

- A C++20 compiler
- CMake 3.16 or later
- Qt 6 Widgets and Test modules for a full build

Run these commands from this directory:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

If CMake cannot find Qt, pass `-DCMAKE_PREFIX_PATH=/path/to/Qt` when configuring to select your Qt installation.
For tests without a display, set `QT_QPA_PLATFORM=offscreen` when running CTest.

A full build produces the library, six examples, and the test executable. Each example can also be built on its own, requiring Qt Widgets:

```bash
cmake -S examples/basic_form -B build-basic_form
cmake --build build-basic_form
```

## Examples

| Directory | Executable target | Description |
|---|---|---|
| `examples/basic_form` | `anker_basic_form` | A basic form with a label, editor, and buttons |
| `examples/relative_widgets` | `anker_relative_widgets` | Position widgets relative to other widgets |
| `examples/position_anchors` | `anker_position_anchors` | Divide a form using proportional positions |
| `examples/opposite_edges` | `anker_opposite_edges` | Align edges of multiple widgets |
| `examples/resizable_dialog` | `anker_resizable_dialog` | A dialog that adapts to resizing |
| `examples/full_attachments` | `anker_full_attachments` | Combine attachment types and spacing |

In a full build, examples are generated under `build/examples/<example-name>/`. On macOS they are `.app` bundles. For example, launch the basic form with:

```bash
open build/examples/basic_form/anker_basic_form.app
```

Code for interactive use with Qt Cling / icpp is included in `iccp/`. See the [icpp examples](iccp/README.en.md) for instructions. Interactive use requires Qt Cling and icpp separately.

## Guides and Distribution

- [User Guide in English](USER_GUIDE.en.md)
- [Original User Guide in Japanese](USER_GUIDE.md)
- [Illustrated User Guide in Japanese (PDF)](USER_GUIDE.pdf)
- [Mermaid edition in Japanese (Markdown)](USER_GUIDE_MERMAID.md) / [PDF](USER_GUIDE_MERMAID.pdf)
- [Original distribution ZIP](anker-layout.zip)

The standard Markdown guide uses genpdf's `!dot` syntax. See the PDFs for the rendered diagrams.

This directory contains the source, examples, tests, and guides extracted from the Sites distribution ZIP. The ZIP and PDFs are preserved unchanged from that distribution. The public READMEs, English translations, and instructions in `iccp/` have been added or adapted for this repository.

Avoid circular attachments. Set initial geometry for `attachSelf()` before calling `addWidget()`. Depending on the offset sign, `attachOppositeForm()` can position a widget outside the form; see the guide for details.
