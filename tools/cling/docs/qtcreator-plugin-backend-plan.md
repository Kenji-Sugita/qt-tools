# Qt Creator plugin and qtcling backend plan

## Position

This is a future plan, not the current implementation target.

The current prototypes are:

- `qtcling`: terminal REPL using Cling `LineEditor`
- `qtcling-gui`: Qt GUI application with an embedded Cling REPL surface

The future Qt Creator integration should reuse the same qtcling direction, but
avoid implementing a full editor from scratch.

## Motivation

Qt Creator already provides the editor, project model, C++ source navigation,
Qt Kit selection, and normal Qt developer workflow.

A Qt Creator plugin could let users send code from the editor to a live qtcling
session and inspect the result without building a standalone editor inside
`qtcling-gui`.

This is especially attractive because `LineEditor` is already good for the
terminal REPL, while a GUI REPL would otherwise need its own history, completion,
multi-line editing, key bindings, and source integration.

Qt Creator's Python project support is a useful reference point. Python
projects can be opened through `pyproject.toml`, and Qt for Python tooling uses
a project file together with helper commands such as `pyside6-project`.
qtcling can follow the same pattern with `qtcling.toml` and `qtcling-project`.

## Preferred architecture

Do not embed Cling directly inside the Qt Creator process as the first design.

Use a separate backend process:

```text
Qt Creator plugin
  - editor integration
  - selected text / current line / current file actions
  - result panel
  - restart button
        |
        | stdio JSON lines, QLocalSocket, or JSON-RPC
        v
qtcling-server
  - owns Cling interpreter
  - owns Qt runtime / event loop
  - eval / load / include / reset commands
  - returns diagnostics and captured output
        |
        v
Cling + Qt
```

## Why separate process

Cling executes user code and JIT-generated code. That code can crash.

If Cling is embedded directly in Qt Creator, a crash can take down the entire
IDE. With a separate `qtcling-server`, a crash usually only kills the backend.
The plugin can detect the disconnect and offer restart.

This also keeps the door open for other clients later, such as VS Code, Vim,
Emacs, or small custom tools.

## Process roles

### Qt Creator plugin

- Adds actions such as "Evaluate Selection", "Evaluate Line", "Load File", and
  "Reset Session".
- Shows output, diagnostics, and status in a dock or output pane.
- Uses Qt Creator's existing editor instead of implementing a new editor.
- May read Qt Kit / project settings and pass them to the backend.
- May read `qtcling.toml` and show its `Sources`, `Headers`, `Forms`,
  `Resources`, and `Translations` in the project tree.
- Should survive backend crashes.

### qtcling-server

- Starts a Cling interpreter.
- Applies Qt configuration: Qt root, modules, include paths, library paths, and
  preload / library load.
- Owns the Qt event loop when needed.
- Accepts commands from clients.
- Returns structured results.
- Can be restarted without restarting the IDE.

## Communication options

### Phase 1: stdio JSON lines

Good for an early prototype.

- Easy to start as a child process.
- Easy to log and debug.
- Works cross-platform.
- One JSON object per line is enough for MVP.

Example request:

```json
{"id":1,"method":"eval","params":{"code":"auto b = new QPushButton(\"OK?\"); b->show();"}}
```

Example response:

```json
{"id":1,"ok":true,"stdout":"","diagnostics":[]}
```

### Phase 2: QLocalSocket

Better for Qt Creator plugin integration.

- Qt-native API.
- Works on macOS, Linux, and Windows.
- Allows a backend process to be restarted and reconnected.

### Phase 3: JSON-RPC

Useful once multiple clients or richer tooling are expected.

## Candidate commands

- `configure`
  - Qt root, module list, Cling root, resource dir, project include paths.
- `eval`
  - Evaluate a line or selected code.
- `loadFile`
  - Equivalent to `.L file.cpp`.
- `addIncludePath`
  - Equivalent to `.I path`.
- `reset`
  - Restart interpreter session.
- `objects`
  - Return known Qt objects if an object registry is added later.
- `ping`
  - Health check.
- `shutdown`
  - Graceful backend exit.

## Relationship to existing tools

`qtcling` should remain the terminal REPL. It benefits from Cling `LineEditor`
and is already comfortable for command-line use.

`qtcling-gui` should remain a standalone GUI REPL prototype. It is useful for
testing a LineEditor-free path and for Windows feasibility.

`qtcling-server` would be a future headless backend shared by IDE plugins and
possibly `qtcling-gui`.

## Risks

- Qt Creator plugin API changes between versions.
- Cling and Qt Creator may require different compiler or Qt runtime assumptions.
- Passing Qt Kit / project configuration into Cling must be carefully designed.
- User code can hang, not just crash. The plugin needs timeout / interrupt /
  restart behavior.
- Windows support needs special attention for MSVC, Windows SDK, Qt DLLs, and
  Clang resource directory discovery.

## MVP

The first useful MVP should be small:

1. `qtcling-server` started as a child process by a test client.
2. JSON-lines `eval` command.
3. macOS Qt root / modules configuration reused from current qtcling work.
4. Backend crash or exit detected by the client.
5. Manual restart.

Qt Creator plugin work should start after the backend protocol is proven with a
small standalone client.
