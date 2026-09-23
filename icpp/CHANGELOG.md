# Changelog

## 1.2.0 - 2026-09-02

### Changed

- Embedded the Streamable HTTP MCP protocol handling in `icpp`.
- Removed the build-time and runtime dependency on the external QtMcpServer
  source tree and shared library.

### Fixed

- Simplified source distribution and installation by shipping MCP support in
  the `icpp` executable.

## 1.1.0 - 2026-08-15

### Added

- Added an authenticated local Streamable HTTP MCP server.
- Added MCP tools for session status, persistent C++/Qt evaluation, and
  interpreter reset.
- Added CMake install rules for `icpp`, QtMcpServer, and the version file.

### Changed

- The root `VERSION` file is now the single source for the CMake project
  version and `icpp --version`.
- Arbitrary MCP code execution requires the explicit
  `--mcp-allow-execution` option.

### Fixed

- Prevented incomplete source with unmatched braces from leaving the
  interpreter in a continued-input state.
- Added install RPATHs on macOS for QtMcpServer and the linked Qt frameworks.

## 1.0.1 - 2026-06-01

### Changed

- Improved `.ptype` output for Qt string view types.
- `QLatin1StringView` now keeps the public C++ type name and shows the Qt meta
  type name only as additional context.
- `QUtf8StringView` now hides the internal `QBasicUtf8StringView` implementation
  name in `.ptype` output.
- Updated the user guide and regenerated the PDF.

## 1.0.0 - 2026-05-20

### Added

- Linux and Windows WSL2 validation notes.
- libedit detection for line editing on Ubuntu 24.04 / WSL2.
- Runtime PropertyEditor discovery from paths relative to the `icpp` executable.

### Changed

- Promoted `icpp` to 1.0.0 alongside `qtcling` 1.0.0.
- Documented WSL2 locale, `libedit-dev`, and `.inspect` deployment layout.
- `.inspect` now supports Linux `property_editor.so` and direct `include/*.h`
  layouts.

## 0.3.0 - 2026-05-12

### Added

- Contextual command help:
  - `.command -h`
  - `.command --help`
  - `.help <command>`
- Workflow/navigation commands:
  - `.examples`
  - `.where`
  - `.runorder`
  - `.qt`
- `.new <kind> [base]` as an alias for `.template <kind> [base]`.
- Starter templates for QWidget, Q_OBJECT, Designer UI, and resource examples.
- VS Code + Copilot guide and Qt Creator guide.

### Changed

- `.b` / `.buffer` defaults to off. The prompt shows `+b` when normal input is
  appended to the edit buffer.
- `.template` generated code now places `{` on cling-friendly definition lines.
- Short command help is printed directly; longer help uses the pager.
- Documentation now recommends matching the Qt 6.x.y version used by `icpp`,
  `qtcling`, `run_all`, Qt tools, QtUiTools, and PropertyEditor.

## 0.2.0 - 2026-05-09

### Added

- Default engine is `qtcling`.
- `--engine cling` and `--engine qtcling` selection.
- `.e` / `.edit` external editor workflow.
- `.r` / `.run` restart workflow.
- Multi-file workflow:
  - `.add <file>`
  - `.files`
  - `.drop <file|number>`
  - `.clearfiles`
- Clipboard workflow through external commands:
  - `.paste`
  - `.copy`
  - `ICPP_CLIPBOARD_PASTE`
  - `ICPP_CLIPBOARD_COPY`
- Value inspection:
  - `.p` / `.print`
  - `.ptype`
- Qt widget helpers:
  - `.widgets`
  - `.closeall`
- Unsaved edit buffer confirmation on `.q` / `.quit`.
- `examples/` with small Qt/C++ samples for `icpp`.
- `USER_GUIDE.md` and `USER_GUIDE.pdf`.
- Internal comparison notes for `ROOT + cling` and `qtcling + icpp`.

### Changed

- `.r` now restarts the interpreter, evaluates registered files in order, then
  evaluates the edit buffer.
- Help output is grouped by workflow.
- `USER_GUIDE.pdf` includes page numbers.

### Notes

- `icpp` remains a lightweight wrapper. It does not replace CMake projects or
  compile source files like ACLiC.
- The intended workflow is to use `icpp` for small Qt/C++ experiments, then move
  stable code into a regular CMake project.

## 0.1.0 - 2026-05-09

Initial internal prototype.
