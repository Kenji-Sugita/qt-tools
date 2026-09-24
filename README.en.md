# Qt Tools

A collection of Qt/C++ development tools, interactive programming environments,
document creation tools, application development workflows, document templates,
work context templates, and everyday utilities.

The repository primarily contains Qt applications and libraries, along with
Python and Bash tools and resources for AI-assisted Qt development.

## Featured: Interactive Qt/C++

These tools let you experiment with C++/Qt interactively and inspect and modify
objects in a running application.

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

The four tools can be used together. ObjectSelector and PropertyEditor are also
available as standalone libraries.

### Qt Cling (`cling`)

An environment for using Qt interactively through Cling.

Qt Cling integrates Qt initialization and event processing with Cling, allowing
you to create and manipulate `QObject` and `QWidget` instances directly from a
terminal.

```cpp
QPushButton *button = new QPushButton("Hello");
button->resize(200, 80);
button->show();
```

It lets you try ordinary Qt/C++ code without repeatedly performing a full
compile-and-link cycle.

### icpp (`icpp`)

A REPL wrapper that adds practical workflows to Cling and Qt Cling.

icpp does not interpret C++ itself. It launches cling or qtcling as a subprocess.

Key features:

- Keep multiple lines of C++ code in an editing buffer
- Edit functions and classes in an external editor
- Rerun edited code from the beginning
- Register multiple files and rerun them together
- Exchange code through the clipboard
- Save experimental code to files
- Display Qt values conveniently
- Use Qt's moc, uic, and rcc tools
- Evaluate code, query session status, and reset the interpreter through MCP

Running Cling in a separate process allows icpp itself to remain running if
experimental C++ code crashes Cling, so you can continue working.

### ObjectSelector (`objectselector`)

A Qt Widgets library for selecting `QObject` and `QWidget` instances within a
Qt application.

- Pick a `QWidget` directly on screen
- Select an object from an object tree
- Highlight the selected target

ObjectSelector handles selection only. It does not display or edit the selected
object's properties.

### PropertyEditor (`propertyeditor`)

A Qt Widgets library for viewing and editing the properties of live Qt objects
using `QObject`, `QMetaObject`, and `QMetaProperty`.

It displays meta-properties and dynamic properties, with editors suited to each
value type.

Representative supported types and property categories:

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

Use it with ObjectSelector, or pass any `QObject` directly to use it independently.

## All Tools

### Interactive C++ / Qt

| Directory | Tool | Description |
|---|---|---|
| [cling](cling/README.txt) | Qt Cling / qtcling | Run Qt/C++ interactively through Cling. |
| [icpp](icpp/README.md) | icpp | A REPL wrapper that adds practical workflows to Cling and Qt Cling. |
| [qmlprop](qmlprop/README.md) | qmlprop | Inspect QML type properties, methods, enums, inheritance, and related information. |

### Qt Object Inspection

| Directory | Tool | Description |
|---|---|---|
| [objectselector](objectselector/USER_GUIDE.md) | ObjectSelector | Select `QObject` and `QWidget` instances on screen or from an object tree. |
| [propertyeditor](propertyeditor/README.md) | PropertyEditor | View and edit `QObject` properties. |

### Development Workflows, Instructions and Templates

| Directory | Tool | Description |
|---|---|---|
| [app-generation-workflow](app-generation-workflow/app-generation-workflow/README.md) | App generation workflow | A document-driven AI development harness focused on GUI applications. It connects requirements, specifications, implementation, testing, and review through documents. |
| [ai-cmake](ai-cmake/USER_GUIDE.md) | AI CMake instructions | Instructions and a validation environment for generating and updating Qt 6 `CMakeLists.txt` files with generative AI. |
| [work-templates](work-templates/README.md) | Work context templates | Keep current work, long-term tasks, and design decisions in separate Markdown files so work can resume across people, machines, and AI sessions. |

These resources support the development process (`app-generation-workflow`),
build definitions (`ai-cmake`), and continuity between work sessions
(`work-templates`). Both app-generation-workflow and work-templates can also be
used for projects that do not use Qt.

### Documents, Images and Diagrams

| Directory | Tool | Description |
|---|---|---|
| [markdown](markdown/USER_GUIDE.md) | genpdf | Generate PDF documents and slides from Markdown. |
| [cutter](cutter/USER_GUIDE.md) | cutter | Crop images using numeric coordinates and dimensions, with support for making backgrounds transparent. |
| [whiteboard](whiteboard/whiteboard-app/USER_GUIDE.md) | WhiteboardApp | Create and edit explanatory diagrams on a whiteboard with multiple pages, PNG/SVG export, and MCP support. |

### Command-line Utilities

| Directory | Tool | Description |
|---|---|---|
| [dateorder](dateorder/USER_GUIDE.md) | younger / older | Check whether a sequence of files is ordered by modification time. |
| [dtree](dtree/USER_GUIDE.md) | dtree | Display directory contents as a tree with branch lines. |
| [now](now/USER_GUIDE.md) | now | Display the current date and time in various formats. |
| [tl](tl/USER_GUIDE.md) | tl | Detect Japanese or English input and translate it into the other language. |

### Desktop Tools and Experiments

| Directory | Tool | Description |
|---|---|---|
| [seminartimer](seminartimer/USER_GUIDE.md) | SeminarTimer | A countdown timer for seminars, exercises, breaks, and similar activities. |
| [qeyes](qeyes/README.md) | Qeyes | A desktop accessory that reimplements classic Xeyes with Qt Widgets. |
| [vu-stereo](vu-stereo/README.md) | VuStereo | Display macOS system playback audio levels on analog-style VU meters. |

## Documentation

For usage instructions, build steps, and requirements, see the `README.md`,
`README.txt`, or `USER_GUIDE.md` in each tool's directory.

For the application development workflow, start with its
[README](app-generation-workflow/app-generation-workflow/README.md) and
[User Guide](app-generation-workflow/app-generation-workflow/USER_GUIDE.md).

Supported operating systems, Qt versions, and dependencies on external libraries
or services vary by tool. Many of the linked documents are in Japanese.

## Platforms

Platform support varies by tool. The collection primarily targets macOS and
Linux, with Qt 6 used by the Qt-based tools.

Some tools have specific requirements:

- VuStereo is macOS-only.
- `tl` requires authentication credentials for Google Cloud Translation.
- Qt Cling and icpp require Cling and a compatible Qt environment.

See each tool's documentation for details.

## About This Repository

Many of these tools grew out of practical needs in Qt/C++ development, seminars,
and document preparation, and have been refined through everyday use.

The focus is on tools that work independently and can be combined as needed,
rather than on building a large integrated environment.
