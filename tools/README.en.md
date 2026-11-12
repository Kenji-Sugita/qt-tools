# Tools

[日本語](README.md) / [Qt Tools](../README.en.md)

Tools to run and libraries to integrate into applications. See each guide for the project layout and build procedure.

Relative paths in this index start from this directory. Run individual builds from the relevant project directory.

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
| [cling](cling/README.en.txt) | Qt Cling / qtcling | Run Qt/C++ interactively through Cling. |
| [icpp](icpp/README.md) | icpp | A REPL wrapper for practical use of Cling and Qt Cling. |

### Qt Object Inspection

| Directory | Tool | Description |
|---|---|---|
| [objectselector](objectselector/USER_GUIDE.en.md) | ObjectSelector | Select `QObject` and `QWidget` instances on screen or from an object tree. |
| [propertyeditor](propertyeditor/README.en.md) | PropertyEditor | View and edit `QObject` properties. |

### Qt Widgets Layouts

| Directory | Tool | Description |
|---|---|---|
| [anker-layout](anker-layout/README.en.md) | Anker Layout | A `QLayout` implementation providing Motif `XmForm`-style attachments for Qt Widgets. Position widgets relative to form edges, other widgets, or proportional positions. |

### QML Tools

| Directory | Tool | Description |
|---|---|---|
| [qmlprop](qmlprop/README.en.md) | qmlprop | Inspect the properties, methods, enumerators, inheritance relationships, and other details of QML types. |
| [qmlprofileranalyzer](qmlprofileranalyzer/USER_GUIDE.en.md) | qmlprofileranalyzer | A command-line tool for analyzing QML Profiler `.qtd` and `.qzt` traces. Inspect summaries, hotspots, and event hierarchies, compare two traces, and convert between the two formats. |

### Documents, Images and Diagrams

| Directory | Tool | Description |
|---|---|---|
| [markdown](markdown/USER_GUIDE.en.md) | genpdf | Generate PDF documents and slides from Markdown. |
| [cutter](cutter/USER_GUIDE.en.md) | cutter | Crop images using numeric parameters, with support for making backgrounds transparent. |
| [whiteboard](whiteboard/whiteboard-app/USER_GUIDE.en.md) | WhiteboardApp | A whiteboard for creating and editing explanatory diagrams, with support for multiple pages, PNG/SVG export, and MCP. |

### General-purpose Command-line Utilities

| Directory | Tool | Description |
|---|---|---|
| [dateorder](dateorder/USER_GUIDE.en.md) | younger / older | Check whether files are ordered by modification time. |
| [dtree](dtree/USER_GUIDE.en.md) | dtree | Display a directory structure as a tree with branch lines. |
| [now](now/USER_GUIDE.en.md) | now | Display the current date and time in various formats. |
| [tl](tl/USER_GUIDE.en.md) | tl | Automatically detect Japanese or English and translate into the other language. |

### Presentation and Seminar Tools

| Directory | Tool | Description |
|---|---|---|
| [LaserPointer](LaserPointer/README.en.md) | LaserPointer | Highlight areas on screen with a laser spot, with dragging, cursor tracking, blinking, and trails. |
| [seminartimer](seminartimer/USER_GUIDE.en.md) | SeminarTimer | A timer that displays the time remaining for seminars, exercises, breaks, and similar activities. |

For creating explanatory diagrams and sharing them on screen, see WhiteboardApp under [Documents, Images and Diagrams](#documents-images-and-diagrams).

### Desktop Accessories

| Directory | Tool | Description |
|---|---|---|
| [qeyes](qeyes/README.en.md) | Qeyes | A desktop accessory that reimplements the classic Xeyes using Qt Widgets. |
| [vu-stereo](vu-stereo/README.en.md) | VuStereo | Display macOS system audio playback levels on analog-style VU meters. |
