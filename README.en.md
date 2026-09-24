# Qt Tools

This repository brings together Qt/C++ development tools, interactive programming
environments, document creation tools, application development workflows and
document templates, templates for managing work context, and everyday utilities.

Most of the applications and libraries use Qt. The repository also includes
Python and Bash tools, along with files that support Qt development using
generative AI.

## Featured: Interactive Qt/C++

These tools go beyond building and running C++/Qt code: they let you experiment
interactively and inspect and modify objects while an application is running.

```text
          Qt Cling
       Qt/C++ GUI REPL
             │
             ▼
           icpp
    practical REPL frontend


   Running Qt application
             │
             ▼
      ObjectSelector
    select QObject/QWidget
             │
             ▼
      PropertyEditor
    inspect/edit QObject
```

All four tools can be used together. ObjectSelector and PropertyEditor can also
be used as independent libraries.

### Qt Cling (`cling`)

An environment for working with Qt interactively through Cling.

Qt Cling combines Qt initialization and event processing with Cling, letting you
create `QObject` and `QWidget` instances from a terminal and manipulate them
immediately.

```cpp
QPushButton *button = new QPushButton("Hello");
button->resize(200, 80);
button->show();
```

Its aim is to let you try out ordinary Qt/C++ code without repeatedly compiling
and linking it.

### icpp (`icpp`)

A REPL wrapper for practical use of Cling and Qt Cling.

icpp does not interpret C++ itself; it launches cling or qtcling as a subprocess.

Key features:

- Work with multiline C++ code in an editing buffer
- Edit functions and classes in an external editor
- Rerun edited code from the beginning
- Register multiple files and rerun them together
- Copy and paste code through the clipboard
- Save code you have tried to a file
- Easily display values of Qt types
- Use Qt's moc, uic, and rcc tools
- Evaluate code, retrieve state, and perform resets through MCP

Keeping Cling in a separate process allows icpp to stay running even if C++ code
executed interactively crashes Cling, so you can continue working.

### ObjectSelector (`objectselector`)

A Qt Widgets library for selecting `QObject` and `QWidget` instances within a
Qt application.

- Select a `QWidget` by pointing to it directly on screen
- Select objects from an object tree
- Highlight the selection target

ObjectSelector handles selection only. It does not display or edit the selected
object.

### PropertyEditor (`propertyeditor`)

A Qt Widgets library that uses `QObject`, `QMetaObject`, and `QMetaProperty` to
view and edit the properties of Qt objects in a running application.

It displays properties exposed through `QMetaProperty` as well as dynamic
properties, and lets you change their values using editors suited to each type.

Supported types and properties include:

- `bool`
- `int`
- `double`
- `QString`
- `QStringList`
- Enums and flags
- `QPoint`
- `QSize`
- `QRect`
- `QColor`
- `QByteArray`
- Dynamic properties

You can combine it with ObjectSelector or use it independently by passing any
`QObject` directly.

## All Tools

### Interactive C++ / Qt

| Directory | Tool | Description |
|---|---|---|
| [cling](cling/README.txt) | Qt Cling / qtcling | Run Qt/C++ interactively through Cling. |
| [icpp](icpp/README.md) | icpp | A REPL wrapper for practical use of Cling and Qt Cling. |

### Qt Object Inspection

| Directory | Tool | Description |
|---|---|---|
| [objectselector](objectselector/USER_GUIDE.md) | ObjectSelector | Select `QObject` and `QWidget` instances on screen or from an object tree. |
| [propertyeditor](propertyeditor/README.md) | PropertyEditor | View and edit `QObject` properties. |

### QML Tools

| Directory | Tool | Description |
|---|---|---|
| [qmlprop](qmlprop/README.md) | qmlprop | Inspect the properties, methods, enumerators, inheritance relationships, and other details of QML types. |
| [qmlprofileranalyzer](qmlprofileranalyzer/USER_GUIDE.md) | qmlprofileranalyzer | A command-line tool for analyzing QML Profiler `.qtd` and `.qzt` traces. Inspect summaries, hotspots, and event hierarchies, compare two traces, and convert between the two formats. |

### Development Workflows, Instructions and Templates

| Directory | Tool | Description |
|---|---|---|
| [app-generation-workflow](app-generation-workflow/app-generation-workflow/README.md) | App generation workflow | A document-driven AI development harness focused on GUI applications. It uses documents to connect requirements gathering, specification, implementation, testing, and review. |
| [ai-cmake](ai-cmake/USER_GUIDE.md) | AI CMake instructions | Instructions and a validation environment for generative AI to create and modify Qt 6 `CMakeLists.txt` files. |
| [work-templates](work-templates/README.md) | Work context templates | Manage work status, long-term tasks, and design decisions separately in Markdown so work can resume when the person, host, or AI session changes. |

These resources support the development process (`app-generation-workflow`),
the creation of build definitions (`ai-cmake`), and continuity of work
(`work-templates`), respectively. Both app-generation-workflow and work-templates
can also be used in projects that do not use Qt.

### Documents, Images and Diagrams

| Directory | Tool | Description |
|---|---|---|
| [markdown](markdown/USER_GUIDE.md) | genpdf | Generate PDF documents and slides from Markdown. |
| [cutter](cutter/USER_GUIDE.md) | cutter | Crop images using numeric parameters, with support for making backgrounds transparent. |
| [whiteboard](whiteboard/whiteboard-app/USER_GUIDE.md) | WhiteboardApp | A whiteboard for creating and editing explanatory diagrams, with support for multiple pages, PNG/SVG export, and MCP. |

### Command-line Utilities

| Directory | Tool | Description |
|---|---|---|
| [dateorder](dateorder/USER_GUIDE.md) | younger / older | Check whether files are ordered by modification time. |
| [dtree](dtree/USER_GUIDE.md) | dtree | Display a directory structure as a tree with branch lines. |
| [now](now/USER_GUIDE.md) | now | Display the current date and time in various formats. |
| [tl](tl/USER_GUIDE.md) | tl | Automatically detect Japanese or English and translate into the other language. |

### Desktop Tools and Experiments

| Directory | Tool | Description |
|---|---|---|
| [seminartimer](seminartimer/USER_GUIDE.md) | SeminarTimer | A timer that displays the time remaining for seminars, exercises, breaks, and similar activities. |
| [qeyes](qeyes/README.md) | Qeyes | A desktop accessory that reimplements the classic Xeyes using Qt Widgets. |
| [vu-stereo](vu-stereo/README.md) | VuStereo | Display macOS system audio playback levels on analog-style VU meters. |

## Documentation

For detailed usage instructions, build steps, and system requirements, see the
`README.md`, `README.txt`, `USER_GUIDE.md`, or other documentation in each tool's
directory.

For the application development workflow, start with the
[README](app-generation-workflow/app-generation-workflow/README.md) and
[User Guide](app-generation-workflow/app-generation-workflow/USER_GUIDE.md).

Supported operating systems, Qt versions, and dependencies on external libraries
and services vary by tool.

## Platforms

Supported environments vary by tool.

The main target platforms and framework version are:

- macOS
- Linux
- Windows
- Qt 6

The command-line tools target macOS and Linux, with a few exceptions such as
genpdf.

Some tools have specific restrictions or requirements:

- VuStereo is available only on macOS.
- `tl` requires authentication to be configured for Google Cloud Translation.
- Qt Cling and icpp require Cling and a compatible Qt environment.

See each tool's documentation for details.

## About This Repository

Many of these tools were created to meet practical needs in Qt/C++ development,
seminars, and document preparation, and have been improved through continued use.

The emphasis is on tools that can be used independently and combined as needed,
rather than on building a large integrated environment.
