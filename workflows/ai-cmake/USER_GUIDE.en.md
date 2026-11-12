---
genpdf:
  format: book
  title: CMake for Generative AI
  subtitle: Instruction File User Guide
  author: SRA, Inc.
  date: 2026-05-07
  font_size: 11pt
  page_numbers: true
---

# CMake Instruction File User Guide for Generative AI

This guide explains how to use `agents/AGENTS.md` and `agents/AGENTS.cmake.md` to
standardize how generative AI creates and modifies Qt 6 `CMakeLists.txt` files.

Examples primarily use Codex, but the CMake policies in `AGENTS.cmake.md` also
apply to other generative AI tools. When using another tool, have it read
`AGENTS.cmake.md` and ask it to follow those instructions when creating or
modifying `CMakeLists.txt`.

## 1. Purpose

This repository contains instruction files that align the policies used for
AI-generated or AI-updated `CMakeLists.txt` files.

This guide explains:

- Each file's role
- Basic placement
- How to make requests to generative AI
- The 33 patterns in `AGENTS.cmake.md`
- How to check generated `CMakeLists.txt` files
- Common troubleshooting steps

## 2. Why Use Instruction Files?

Asking an AI to generate a `CMakeLists.txt` each time may be enough for a one-off task.

In areas such as Qt / CMake, however, multiple valid approaches and a mixture
of old and new styles can make output inconsistent. Even when an AI updates
`CMakeLists.txt` after adding code, it may not apply the same approach each time.

Instruction files guide its decisions toward your standard policies.

### 2.1 Compared with Ad Hoc Requests

Individual requests let you describe the immediate conditions flexibly. However,
decisions such as using `qt_standard_project_setup()`, registering QML through
`qt_add_qml_module()`, or using `.qrc` may vary between requests.

Instruction files record a standard approach in advance so the same rules can
be reused across projects.

### 2.2 Benefits

- Reduce output variation.
- Establish modern Qt 6 CMake policies.
- Avoid repeating long prompts.
- Apply the same criteria to modification and review of existing files.
- Reduce ad hoc decisions when updating build definitions after adding code.
- Make it easier to create a starting point for migrating qmake `.pro` files to Qt 6 / CMake.
- Share CMake policies across repositories.

### 2.3 Caveats

- Update `AGENTS.cmake.md` as needed.
- State exceptions for unusual projects in the request or repository `AGENTS.md`.
- Overly rigid instructions may not fit project-specific circumstances.
- Always build to verify the generated result.

### 2.4 When to Use Each Approach

Direct requests may be sufficient for one-off experiments or small samples.

Instruction files are better suited to cases where you:

- Work repeatedly with Qt / CMake projects.
- Want shared CMake policies across repositories.
- Want to avoid older Qt CMake styles.
- Want consistent QML registration.
- Want consistent review criteria.
- Want consistent updates after code additions.
- Are migrating from qmake to CMake.

### 2.5 Suggested Practice

Use instruction files as the default and state only exceptions in your requests.

When a repository containing `AGENTS.md` is open in Codex, even a short request
can help produce a `CMakeLists.txt` following `AGENTS.cmake.md`:

```bash
codex "Create a CMakeLists.txt"
```

Examples:

```text
Follow AGENTS.md and ~/AGENTS.cmake.md to create CMakeLists.txt.
This project must support Qt 6.2.
```

```text
Follow AGENTS.md and ~/AGENTS.cmake.md.
However, do not change the existing qrc:/... loading URLs.
```

```text
Update CMakeLists.txt for the added code.
Follow AGENTS.md and ~/AGENTS.cmake.md when updating it.
```

The files carry the standard policies, leaving you to explain only project-specific circumstances.

## 3. Overview

### 3.1 Files in This Repository

The two main files are:

```text
agents/
├── AGENTS.md
└── AGENTS.cmake.md
```

`AGENTS.md` provides basic instructions for the working repository.

`AGENTS.cmake.md` provides detailed instructions for creating and modifying Qt 6
`CMakeLists.txt` files.

### 3.2 Relationship Between AGENTS.md and AGENTS.cmake.md

In the standard setup, each repository has `AGENTS.md`, while `AGENTS.cmake.md`
is placed in the home directory.

`AGENTS.md` directs the AI to read `~/AGENTS.cmake.md` before creating, modifying,
or updating `CMakeLists.txt`. This shares the same generation rules across repositories.

### 3.3 How the AI Reads the Instructions

Basic Codex workflow:

1. Codex reads the working repository's `AGENTS.md`.
2. If `CMakeLists.txt` needs creation, modification, or updating, it reads `~/AGENTS.cmake.md`.
3. It follows that file's policies and patterns to generate or modify `CMakeLists.txt`.

Other AI tools may not automatically read these files. Attach the files, paste
their contents, or explicitly ask the tool to read and follow them.

## 4. Basic Placement

### 4.1 Put AGENTS.md at the Repository Root

Place it at the root of the repository where you will work:

```text
your-project/
├── AGENTS.md
├── CMakeLists.txt
└── src/
```

### 4.2 Put AGENTS.cmake.md in the Home Directory

Place it at `~/AGENTS.cmake.md`:

```text
~/
└── AGENTS.cmake.md
```

### 4.3 Example Directory Layout

This separates the working repository from the shared CMake instructions:

```text
~/
└── AGENTS.cmake.md

your-project/
├── AGENTS.md
├── CMakeLists.txt
├── src/
└── tests/
```

## 5. Setup Procedure

### 5.1 Copy AGENTS.md

Copy this repository's `agents/AGENTS.md` to the working repository root.

Example:

```text
agents/AGENTS.md -> your-project/AGENTS.md
```

### 5.2 Install AGENTS.cmake.md as ~/AGENTS.cmake.md

Copy `agents/AGENTS.cmake.md` into your home directory.

Example:

```text
agents/AGENTS.cmake.md -> ~/AGENTS.cmake.md
```

### 5.3 Check the Reference in AGENTS.md

Check that it contains an instruction like:

```markdown
When creating or updating `CMakeLists.txt`, do not decide freely based only on
source code. Always follow the policies and applicable patterns in
`~/AGENTS.cmake.md`. Apply the same rule when updating CMakeLists.txt after adding code.
```

If `AGENTS.cmake.md` is elsewhere, update the reference accordingly.

### 5.4 Make the AI Aware of the Files

In Codex, open the working repository and request creation, modification, or
updating of `CMakeLists.txt` as usual.

For another AI tool, have it read `AGENTS.cmake.md` and follow the same policies.

If needed, make this explicit:

```text
Follow the instructions in AGENTS.md and ~/AGENTS.cmake.md.
```

## 6. Basic Usage

### 6.1 Create a New CMakeLists.txt

If a new Qt project has no `CMakeLists.txt`, describe the project type in the request.

Example:

```text
Create a CMakeLists.txt for this Qt Widgets application.
```

### 6.2 Modify an Existing CMakeLists.txt

When adapting an existing file for Qt 6, explicitly ask to preserve existing behavior.

Example:

```text
Update this CMakeLists.txt to modern Qt 6 CMake while preserving existing behavior.
```

### 6.3 Request a Qt Widgets Configuration

Mentioning `QApplication` or `QMainWindow` helps the AI choose the Widgets pattern.

Example:

```text
This Widgets application uses QApplication and QMainWindow. Create a CMakeLists.txt.
```

### 6.4 Request a Qt Quick / QML Configuration

For QML applications, explain how QML is loaded.

For `loadFromModule()`:

```text
This is a Qt Quick application. Configure it with qt_add_qml_module(), assuming main.cpp uses loadFromModule().
```

For `qrc:/...`:

```text
The existing main.cpp loads qrc:/qt/qml/app/Main.qml. Create a CMakeLists.txt that preserves this URL.
```

### 6.5 Add Tests

For Qt Test, ask to separate test targets from the application itself.

Example:

```text
Add a Qt Test target. Separate the logic under test into a library.
```

### 6.6 Update CMakeLists.txt After Adding Code

Ask the AI to follow the instruction-file policies rather than deciding freely
how to update the file after code additions.

Example:

```text
Inspect the added source files and the Qt classes used, then update CMakeLists.txt.
Follow AGENTS.md and ~/AGENTS.cmake.md when updating it.
```

### 6.7 Migrate from qmake to CMake

Ask for modern Qt 6 CMake rather than a mechanical transcription of `.pro` settings.

Example:

```text
Inspect this .pro file and source layout, then create a Qt 6 CMakeLists.txt.
Preserve existing target names, source files, and Qt modules while following AGENTS.cmake.md.
```

## 7. Example AI Requests

### 7.1 Create a New CMakeLists.txt

With a repository containing `AGENTS.md` open in Codex:

```bash
codex "Create a CMakeLists.txt"
```

Even this short request can follow shared policies because `AGENTS.md` refers
to `~/AGENTS.cmake.md`.

To state project-specific conditions:

```text
Inspect the sources in this directory and create a Qt 6 CMakeLists.txt.
Follow AGENTS.md and ~/AGENTS.cmake.md.
```

### 7.2 Modernize an Existing CMakeLists.txt

```text
Update the existing CMakeLists.txt to modern Qt 6 CMake.
Preserve target names and runtime behavior.
```

### 7.3 Fix QML Resource Configuration

```text
Review QML registration.
Use qt_add_qml_module() or qt_add_resources() to match how main.cpp loads QML.
```

### 7.4 Add Qt Test

```text
Add a Qt Test target.
Configure add_test() so it can run through CTest.
```

### 7.5 Fix Build Errors

```text
Fix this build error.
Limit CMakeLists.txt changes to the minimum needed to address its cause.
```

### 7.6 Update CMakeLists.txt After Code Additions

```text
Inspect the added source files and Qt modules used, then update CMakeLists.txt.
Follow AGENTS.md and ~/AGENTS.cmake.md when updating it.
```

### 7.7 Migrate from qmake to CMake

```text
Inspect the existing .pro file and create a Qt 6 CMakeLists.txt.
Respect the existing structure while following AGENTS.cmake.md to use modern CMake.
```

### 7.8 Review the Generated Result

```text
Review the generated CMakeLists.txt.
Check Qt module declarations, QML registration, and target settings.
```

## 8. The 33 Patterns in AGENTS.cmake.md

### 8.1 Application Patterns

- Qt Widgets application
- Qt Quick / QML application
- Qt console application
- Project with multiple executables

### 8.2 Library Patterns

- Qt Core library
- Qt Widgets library
- Application plus shared code library

### 8.3 Test Patterns

- Test target using Qt Test
- Application plus tests

### 8.4 Subdirectory Structure

- Root `CMakeLists.txt`
- `src/CMakeLists.txt`
- `tests/CMakeLists.txt`
- Layout using `add_subdirectory()`

### 8.5 Qt Module Patterns

- Widgets application using Qt Designer `.ui` files
- Widgets application using resource files
- Application using Qt Network
- Application using Qt SQL
- Application using Qt Concurrent
- Application using OpenGL / Qt OpenGL
- Project with translation files

### 8.6 Distribution and Compatibility Patterns

- Configuration with `install()`
- macOS / Windows GUI application properties
- Configuration supporting Qt versions before 6.3
- Environment with versionless Qt CMake commands disabled

### 8.7 Advanced QML Patterns

- Qt Quick Controls application
- Application with multiple QML files
- Application with QML resources
- Application exposing a C++ backend to QML
- Application using a QML singleton
- QML application preserving existing `qrc:/...` loading
- QML module packaged as a library
- Multiple QML modules
- QML module with C++ types
- QML application plus tests
- QML application installation with `install()`
- QML configuration for Qt versions before 6.3

## 9. Core Policies for Generated CMakeLists.txt Files

### 9.1 Use Modern Qt 6 CMake

`AGENTS.cmake.md` assumes modern Qt 6 CMake. Older Qt 5 styles and dependence
on global settings are generally avoided.

### 9.2 Use qt_standard_project_setup()

When Qt 6.3 or later can be assumed, call `qt_standard_project_setup()` immediately
after `find_package(Qt6 ...)`.

### 9.3 Use qt_add_executable() / qt_add_library()

Prefer `qt_add_executable()` for Qt applications.
Use `qt_add_library()` for Qt libraries as appropriate.

### 9.4 Link Qt Imported Targets

Link modules using imported targets such as `Qt6::Widgets` and `Qt6::Quick`.

### 9.5 Avoid Unnecessary Global Settings

For new files, avoid:

- `include_directories()`
- `link_libraries()`
- Unnecessary `file(GLOB ...)`
- Unnecessary manual `CMAKE_AUTOMOC` / `CMAKE_AUTOUIC`

## 10. Alternative Placement Patterns

### 10.1 AGENTS.cmake.md in the Home Directory

The default arrangement suits shared CMake policies across repositories.

```text
~/
└── AGENTS.cmake.md

your-project/
└── AGENTS.md
```

### 10.2 AGENTS.cmake.md in the Repository

Use this when policies differ by project:

```text
your-project/
├── AGENTS.md
├── AGENTS.cmake.md
└── CMakeLists.txt
```

Change the reference in `AGENTS.md` to `./AGENTS.cmake.md`.

### 10.3 AGENTS.md in a Subdirectory

If a large repository contains Qt projects only in one area, place a dedicated
`AGENTS.md` in that subdirectory:

```text
repository/
├── AGENTS.md
└── qt-app/
    ├── AGENTS.md
    └── CMakeLists.txt
```

### 10.4 All Instructions in AGENTS.md

For small projects, CMake instructions may be included directly in `AGENTS.md`.
Separating `AGENTS.cmake.md` makes updates easier when reusing it across projects.

### 10.5 Use a Different Filename

Match the reference in `AGENTS.md` to the actual instruction filename.

Example:

```markdown
Before creating or updating `CMakeLists.txt`, always read and follow `./docs/CMakeRules.md`.
```

## 11. Checking Generated Results

### 11.1 Check CMake Structure

Verify that the generated file contains at least:

- `cmake_minimum_required()`
- `project()`
- `find_package(Qt6 REQUIRED COMPONENTS ...)`
- Target definitions
- `target_link_libraries()`

### 11.2 Check Required Qt Modules

Ensure modules corresponding to the Qt classes used appear in both
`find_package()` and `target_link_libraries()`.

Examples:

- QWidget classes: `Widgets`
- QML / Qt Quick: `Quick`
- Networking: `Network`
- SQL: `Sql`
- Qt Test: `Test`

### 11.3 Match QML Loading to CMake Registration

Check that QML loading in `main.cpp` matches CMake registration:

- `loadFromModule()`: `qt_add_qml_module()`
- `qrc:/...`: `qt_add_resources()`

### 11.4 Check GUI / Console / Test Target Settings

GUI applications may need these properties:

```cmake
set_target_properties(MyApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

They are normally not set for console applications or test targets.

### 11.5 Build to Verify

Run CMake configure and build:

```text
cmake -S . -B build
cmake --build build
```

With Qt Creator, open the project and verify that Configure and Build succeed.

## 12. Common Problems

### 12.1 The Instructions Are Not Being Read

Check the reference in `AGENTS.md`. In the standard arrangement, it is `~/AGENTS.cmake.md`.

### 12.2 qt_standard_project_setup() Is Not Found

You may be using Qt earlier than 6.3. Ask the AI to use the pattern for
supporting Qt versions before 6.3 in `AGENTS.cmake.md`.

### 12.3 QML Files Are Not Found at Runtime

Check that the loading URL in `main.cpp` matches QML registration in CMake.
`loadFromModule()` and `qrc:/...` use different CMake commands.

### 12.4 Required Qt Modules Are Not Linked

A module corresponding to a Qt class in use may be missing. Give the AI the
complete error and the Qt classes used when requesting a fix.

### 12.5 Failure on Qt Earlier Than 6.3

`qt_standard_project_setup()` was introduced in Qt 6.3. For Qt 6.2 and similar
versions, use the compatibility pattern for Qt versions before 6.3.

### 12.6 Versionless Qt CMake Commands Are Unavailable

Some environments do not provide versionless commands such as `qt_add_executable()`.
Ask for the pattern using `qt6_add_executable()` and `qt6_standard_project_setup()`.

## 13. Practical Tips

### 13.1 Put Project-Specific Rules in AGENTS.md

Record constraints such as target names, directory layout, and supported Qt
versions in the working repository's `AGENTS.md`.

### 13.2 Consolidate Shared Rules in AGENTS.cmake.md

Keep core Qt CMake policies and generation patterns in `AGENTS.cmake.md`.
Reusing the same rules across repositories improves consistency, including
updates to `CMakeLists.txt` after code additions.

### 13.3 State Exceptions in the Request

Explicitly mention conditions such as Qt 6.2, preserving an existing `.qrc`,
or keeping a particular target name.

### 13.4 Ask for a Review After Generation

Have the AI review its generated `CMakeLists.txt` to reduce oversights.

Example:

```text
Review this CMakeLists.txt and check that it follows AGENTS.cmake.md.
```

### 13.5 Updating Rules Shared Across Repositories

Changes to `~/AGENTS.cmake.md` affect AI decisions across multiple repositories.
For major changes, add exceptions or supplementary instructions to project-level
`AGENTS.md` files as needed.

## 14. Appendix

### 14.1 Minimal AGENTS.md Example

```markdown
# AGENTS.md

## Change Policy

- Propose unrequested features or specification changes and obtain permission before implementation.
- When refactoring, prioritize preserving existing specifications and keep changes minimal.

## Additional Instructions

- When creating or updating `CMakeLists.txt`, do not decide freely based only on
  source code. Always follow the policies and applicable patterns in `~/AGENTS.cmake.md`.
- Apply the same rule when updating `CMakeLists.txt` after adding code.
```

### 14.2 Basic Layout Example

```text
~/
└── AGENTS.cmake.md

your-project/
├── AGENTS.md
├── CMakeLists.txt
├── src/
└── tests/
```

### 14.3 AI Request Template

```text
Create a CMakeLists.txt for this project.
Follow AGENTS.md and ~/AGENTS.cmake.md.

Conditions:
- Use Qt 6.
- Application type: <Widgets / Quick / Console>.
- Do not change existing target names.
- Add test targets if needed.
- When migrating from qmake, respect the existing .pro file's structure.
```

### 14.4 List of 33 Patterns

1. Qt Widgets application
2. Qt Quick / QML application
3. Qt console application
4. Qt Core library
5. Qt Widgets library
6. Application plus shared code library
7. Project with multiple executables
8. Test target using Qt Test
9. Application plus tests
10. Subdirectory structure
11. Widgets application using Qt Designer `.ui` files
12. Widgets application using resource files
13. Application using Qt Network
14. Application using Qt SQL
15. Application using Qt Concurrent
16. Application using OpenGL / Qt OpenGL
17. Project with translation files
18. Configuration with `install()`
19. macOS / Windows GUI application properties
20. Configuration supporting Qt versions before 6.3
21. Environment with versionless Qt CMake commands disabled
22. Qt Quick Controls application
23. Application with multiple QML files
24. Application with QML resources
25. Application exposing a C++ backend to QML
26. Application using a QML singleton
27. QML application preserving existing `qrc:/...` loading
28. QML module packaged as a library
29. Multiple QML modules
30. QML module with C++ types
31. QML application plus tests
32. QML application installation with `install()`
33. QML configuration for Qt versions before 6.3
