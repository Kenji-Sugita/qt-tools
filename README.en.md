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

## Featured: App Generation Workflow

`app-generation-workflow` is a document-driven AI development harness focused on
GUI applications. It provides procedures and templates that connect requirements
gathering, specification, implementation, testing, and review.

Describe what you want to build, organize usage scenarios, screens and
interactions, and business rules, then turn them into implementation instructions
for AI. After generation, check behavior against the specifications and document
how the implementation corresponds to them.

### From Requirements to Implementation and Verification

```text
Requirements notes (.txt)
       │ Requirements review
       ▼
Pre-specification notes (source_memo.md)
       │ Organize goals, operations, rules, exclusions, and open questions
       ▼
Three key specifications
  ├─ Use cases      Usage scenarios and expected outcomes
  ├─ UI             Screens, input, display, and interactions
  └─ Business layer Data, validation, calculation, and persistence
       │ Review, then select the implementation approach
       ▼
Implementation prompt → Application implementation
       │ Build, tests, manual checks, and implementation review
       ▼
Record specification-to-code correspondence → Application design document
```

AI drafts documents, identifies gaps and contradictions, implements code, writes
tests, and checks consistency. People decide what to build, which specifications
to adopt, and whether to accept the generated results. Reviews at each stage
check the conditions for proceeding and where to return when changes are needed.

### The Three Key Specifications

The material gathered in the pre-specification notes is organized into three
perspectives.

| Document | File | What it describes |
|---|---|---|
| Use-case specification | `01_usecase_spec.md` | User goals, interaction sequences, success conditions, and behavior in exceptional cases |
| UI specification | `02_ui_spec.md` | Screen layout, input and display, conditions for available operations, and error presentation |
| Business-layer specification | `03_business_spec.md` | UI-independent data, validation, calculation, persistence, and state changes |

For example, for an "open file" feature, the use-case specification defines the
usage sequence and failure behavior, the UI specification defines the open
action and error presentation, and the business-layer specification defines
loading and data validation. Comparing the same feature from these perspectives
helps reveal inconsistencies between the interface and its underlying behavior.

The implementation language and framework are specified in the implementation
prompt after this preparation. The workflow can also be used in projects that do
not use Qt.

### Verification and Revision After Generation

Key verification and revision steps:

- Derive expected test results from the specifications and compare them with the generated code's output
- Perform necessary manual checks, such as screen interactions, alongside automated tests
- When a specification decision changes, update the pre-specification notes, the three specifications, the implementation prompt, and then the code
- When the specifications are correct and only transcription or implementation is faulty, confirm the source specification and return to the affected stage
- Recheck artifacts affected by upstream changes and record both verified results and items that remain unverified
- After implementation review confirms that no major inconsistencies remain, document specification-to-code correspondence and the overall application design

Reuse these documents for subsequent changes, so feature additions and
corrections to misunderstood intent can be made while checking the connections
between requirements, specifications, and implementation.

### Getting Started

Read the [User Guide](app-generation-workflow/app-generation-workflow/USER_GUIDE.en.md).
Copy the repository's inner `app-generation-workflow/app-generation-workflow/`
directory to `app-generation-workflow/` in your working location. Prepare a `.txt`
file containing your requirements (`requirement.txt` in this example), then
start with an instruction to AI such as:

```text
Read app-generation-workflow/three_key_documents_workflow.md.
Review the requirements in requirement.txt.
Identify missing information and open questions before creating the pre-specification notes.
```

Keep application-specific specifications, implementation prompts, and source
code in a separate application directory, apart from the templates. See the
[workflow README](app-generation-workflow/app-generation-workflow/README.en.md)
and [User Guide](app-generation-workflow/app-generation-workflow/USER_GUIDE.en.md)
for the directory structure and detailed procedure.

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

### QML Tools

| Directory | Tool | Description |
|---|---|---|
| [qmlprop](qmlprop/README.en.md) | qmlprop | Inspect the properties, methods, enumerators, inheritance relationships, and other details of QML types. |
| [qmlprofileranalyzer](qmlprofileranalyzer/USER_GUIDE.en.md) | qmlprofileranalyzer | A command-line tool for analyzing QML Profiler `.qtd` and `.qzt` traces. Inspect summaries, hotspots, and event hierarchies, compare two traces, and convert between the two formats. |

### Development Workflows, Instructions and Templates

| Directory | Tool | Description |
|---|---|---|
| [app-generation-workflow](app-generation-workflow/app-generation-workflow/README.en.md) | App generation workflow | A document-driven AI development harness focused on GUI applications. It uses documents to connect requirements gathering, specification, implementation, testing, and review. |
| [ai-cmake](ai-cmake/USER_GUIDE.en.md) | AI CMake instructions | Instructions and a validation environment for generative AI to create and modify Qt 6 `CMakeLists.txt` files. |
| [work-templates](work-templates/README.en.md) | Work context templates | Manage work status, long-term tasks, and design decisions separately in Markdown so work can resume when the person, host, or AI session changes. |

These resources support the development process (`app-generation-workflow`),
the creation of build definitions (`ai-cmake`), and continuity of work
(`work-templates`), respectively. Both app-generation-workflow and work-templates
can also be used in projects that do not use Qt.

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

## Documentation

For detailed usage instructions, build steps, and system requirements, see the
`README.md`, `README.txt`, `USER_GUIDE.md`, or other documentation in each tool's
directory.

For the application development workflow, start with the
[README](app-generation-workflow/app-generation-workflow/README.en.md) and
[User Guide](app-generation-workflow/app-generation-workflow/USER_GUIDE.en.md).

Supported operating systems, Qt versions, and dependencies on external libraries
and services vary by tool.

The root-level `NEXT.md`, `DECISIONS.md`, and `backlog.md` contain the maintainer's
active work context for developing `qt-tools`. They are tracked in Git to share
the current work state, design decisions, and outstanding issues across hosts
and AI sessions. They are not templates for users to adopt as their own work
context after cloning the repository. For reusable templates, see
[work-templates/](work-templates/README.en.md).

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
