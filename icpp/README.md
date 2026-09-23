# icpp

`icpp` is a small REPL wrapper for `cling` and `qtcling`.

It does not interpret C++ by itself. It starts `cling` or `qtcling` as the
interpreter process and adds workflow-oriented commands for editing, rerunning,
multi-file experiments, value inspection, and clipboard exchange.

## Version

The current release is recorded in [`VERSION`](VERSION). This file is the
single source for the CMake project version and `icpp --version`. Update it
before configuring a release build.

## Requirements

- macOS or Linux
- Qt 6
- CMake 3.16 or later
- A working `cling` or `qtcling`
- Optional line editing support: GNU Readline or libedit (`libedit-dev` on Ubuntu)

When using `qtcling`, build and run `icpp` with the same Qt 6.x.y tree used by
`qtcling`, `run_all`, `moc`, `uic`, `rcc`, `QtUiTools`, and the PropertyEditor
support library. Treat `icpp` as Qt-version-specific, similar to Squish builds
for each Qt version.

The default engine is `qtcling`. Typical internal paths are:

```text
/usr/local/src/cling/bin/qtcling
/usr/local/src/cling/build/bin/cling
```

You can override them with command line options or environment variables.

```text
icpp --engine qtcling --qtcling /path/to/qtcling
icpp --engine cling --cling /path/to/cling
```

```text
ICPP_QTCLING=/path/to/qtcling
ICPP_CLING=/path/to/cling
```

icpp messages, `.help` / `.?`, and safety prompts are Japanese by default. Use
English with:

```text
icpp --help-language en
ICPP_HELP_LANGUAGE=en icpp
icpp[qtcling]> .lang en
```

`.lang ja` / `.lang en` changes the language while icpp is running and stores
the choice with QSettings. Command-line options and environment variables still
override the stored setting at startup.

In an interactive terminal, `.help`, `.h`, and `.?` open the help text in a
pager. Set `ICPP_PAGER` or `PAGER` to choose a pager; the default is `less -R`.
Non-interactive use still prints the help text to standard output.
Use `.command -h` for detailed help on a specific command, for example
`.e -h`, `.r -h`, or `.template -h`.

## Build

```text
cmake -S . -B build
cmake --build build
```

Install system-wide under `/usr/local`:

```text
sudo cmake --install build --prefix /usr/local
```

This installs `icpp` to `/usr/local/bin` and the version file to
`/usr/local/share/icpp/VERSION`.

Run:

```text
./build/icpp
```

Check the version:

```text
./build/icpp --version
```

## Streamable HTTP MCP Server

icpp can run as a local Streamable HTTP MCP server. The normal interactive
REPL remains the default; MCP mode starts only when `--mcp-http` is specified.

Set a private token of at least 32 bytes and start the server:

```text
export ICPP_MCP_TOKEN='replace-with-a-long-random-token'
./build/icpp --mcp-http --mcp-port 8766 --engine qtcling
```

The endpoint is:

```text
http://127.0.0.1:8766/mcp
```

Every request must include:

```text
Authorization: Bearer <ICPP_MCP_TOKEN>
```

Without an additional option, the server permits only the read-only status
tool. To allow C++ or Qt code execution, start it explicitly with:

```text
./build/icpp --mcp-http --mcp-port 8766 --mcp-allow-execution
```

This option permits authenticated MCP clients to execute arbitrary native code
with the same operating-system permissions as the icpp process. It is not a
sandbox. Use it only with trusted local clients and trusted code. The server
listens only on loopback, accepts one MCP session, limits requests and captured
output to 1 MiB, and limits each evaluation to at most 30 seconds. The token is
removed from the cling or qtcling child-process environment.

The MCP tools are:

```text
icpp/session/status   show engine, interpreter state, working directory,
                      and whether execution is allowed
icpp/code/evaluate    evaluate code in the persistent interpreter session
icpp/session/reset    restart the interpreter and clear session definitions
```

`icpp/code/evaluate` accepts `source` and an optional `timeoutMs` between 100
and 30000. Definitions remain available to later evaluations in the same MCP
session. A timeout or interpreter crash terminates the child process; the next
evaluation starts a fresh interpreter. Sending HTTP DELETE with the active
`Mcp-Session-Id` ends the session immediately.

The built-in MCP implementation supports Streamable HTTP protocol versions
`2025-03-26` and `2025-06-18`. It returns JSON responses and does not provide a
server-initiated SSE stream; GET `/mcp` returns HTTP 405.

## Typical Workflow

Use `icpp` as a lightweight workbench for Qt/C++ experiments.

```text
icpp[qtcling]> .e scratch.cpp
icpp[qtcling]> .r
icpp[qtcling]> .x static auto w = go();
icpp[qtcling]> .defs
```

For several source files:

```text
icpp[qtcling]> .add SimpleClass.cpp
icpp[qtcling]> .add UseSimpleClass.cpp
icpp[qtcling]> .r
icpp[qtcling]> .p makeValue()
```

Use `.e` without an argument for quick scratch-buffer editing. For regular work with Qt Creator, VS Code, Emacs, or Copilot, prefer real files with `.e <file>` or `.add <file>` so editor history and language tooling stay useful.

For clipboard exchange with an editor or GitHub Copilot:

```text
icpp[qtcling]> .paste
icpp[qtcling]> .r
icpp[qtcling]> .copy
```

## Main Commands

```text
.e / .edit [file|num]   edit the current buffer or a registered file
.r / .run [edit]        restart and evaluate registered files, then buffer
                        use edit to reorder registered files first
.gen                    run run_all, then restart and evaluate
.status / .st           show session status
.doctor                 check interpreter, Qt tools, and current directory
.generated              show generated files in the current directory
.examples               show short workflow examples
.where                  show current directory, buffer, and file summary
.runorder               show registered file evaluation order
.qt                     show compact Qt generated-file status
.clean                  delete generated files after confirmation
.autogen on|off         toggle automatic run_all before reevaluation
.b                      toggle normal input buffer append on/off; default is off
.add <file>             add a file evaluated by .r
.files                  show registered files
.drop <file|number>     remove a registered file
.p <expr>               print an expression
.ptype <expr>           show the expression type
.defs                   show definitions in registered files and the buffer
.a <code>               evaluate C++ code and append it to the buffer
.uiinfo <file.ui>       show widgets, layouts, and actions in a .ui file
.designer/.de <form|file.ui> create or edit a .ui file with Qt Designer
.linguist/.li <file.ts> open a .ts file with Qt Linguist
.qrc [text|creator] <file.qrc> edit a .qrc file
.qtc <file.qrc>        edit a .qrc file with Qt Creator
.template <kind> [base] create starter files; use .template -h for details
.new <kind> [base]      alias for .template
.quiet [on|off]         show or set persistent quiet hints
.newname [lower|asis]   show or set new file name casing
.lang [ja|en]           show or set persistent help language
.preview/.pv <file.ui>  preview a .ui file directly
.inspect                inspect top-level widgets with PropertyEditor
.x <code>               evaluate C++ code without appending to the buffer
.paste                  append clipboard text to the buffer
.copy                   copy the buffer to the clipboard
.! <command>            run an external shell command
.i [path|edit]          show, add, or edit include paths
.widgets                show top-level QWidget objects
.closeall               close top-level QWidget objects
```

Most commands accept `-h` or `--help` for detailed command-specific help.

Normal input is not appended to the edit buffer by default. Use `.b on` when
you want normal input to be kept in the edit buffer. In that mode the prompt
shows `+b`, for example `icpp[qtcling +b]>`.

Use `.x <code>` for one-shot execution that should never be appended to the
edit buffer, regardless of the current `.b` mode.
Use `.a <code>` / `.append <code>` for one-shot execution that should also be
appended to the edit buffer.

See `USER_GUIDE.pdf` for the full guide.
