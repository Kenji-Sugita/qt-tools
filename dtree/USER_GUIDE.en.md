---
genpdf:
  Format: book
  Title: dtree User Guide
  Subtitle: Version 1.0
  Author: SRA, Inc.
---

# 1. Overview

`dtree` is a small command-line tool that displays directory contents as a tree.

It recursively walks the specified directory and displays files and subdirectories
with readable branch lines. You can switch between Unicode and ASCII-only lines,
include hidden files and directories, show the root's full path instead of its
basename, and display counts at the end.

Typical uses include:

- Quickly checking a directory structure
- Including distribution or sample layouts in documentation
- Using ASCII output in environments without Unicode support

# 2. Basic Behavior

`dtree` traverses the starting directory depth-first.

- The starting directory itself appears first.
- Its basename is displayed by default.
- `-f`, `--full-root` displays its full path instead.
- Child entries appear below it in order.
- Normal entries at each level are sorted by name.
- With `--all`, hidden entries follow normal entries, with each group sorted by name.
- Directory names end with `/`.
- Hidden files and directories are excluded by default.
- `--all` includes hidden files and directories.

Without an argument, the current directory (`.`) is displayed.

# 3. Installation

Extract the distribution ZIP:

```text
$ unzip dtree-release-20260416.zip
$ cd dtree-release-20260416
$ chmod +x dtree
```

Run it from the current directory:

```text
$ ./dtree
```

To run it from any directory, place it on `PATH`:

```text
$ cp dtree ~/bin/
$ chmod +x ~/bin/dtree
```

On Windows, use a Bash environment such as Git Bash or WSL instead of relying on shebang execution:

```text
$ bash ./dtree
```

# 4. Running

Typical invocation:

```text
$ dtree
dtree/
└─ dtree
```

You can specify a directory:

```text
$ dtree src
src/
├─ lib/
└─ main.sh
```

Display help:

```text
$ dtree --help
```

You can also run the script directly with Bash:

```text
$ bash ./dtree .
```

# 5. First Examples

## 5.1 Displaying the Current Directory

```text
$ dtree
dtree/
├─ docs/
├─ dtree
└─ samples/
```

Without arguments, the current directory is the starting point.

## 5.2 Displaying a Specified Directory

```text
$ dtree project
project/
├─ bin/
├─ lib/
└─ README.md
```

Specify one directory name to use it as the starting point.

# 6. Basic Options

`dtree` lets you change the branch-line style.

## 6.1 Help

```text
$ dtree --help
```

- `-h`, `--help` displays usage information.

## 6.2 Line Style

```text
$ dtree --lines unicode
$ dtree --lines ascii
$ dtree --unicode
$ dtree --ascii
```

- `--lines unicode` uses Unicode branch lines.
- `--lines ascii` uses ASCII characters `|`, `+`, and `-`.
- `--unicode` is equivalent to `--lines unicode`.
- `--ascii` is equivalent to `--lines ascii`.

## 6.3 Hidden Files

```text
$ dtree --all
$ dtree -a project
```

- `-a`, `--all` also displays files and directories whose names begin with `.`.
- Both normal and hidden entries are sorted by name.
- Hidden entries follow normal entries.

## 6.4 Counts

```text
$ dtree -s project
project/
├─ bin/
│  └─ run.sh
├─ lib/
│  ├─ core.sh
│  └─ util.sh
└─ README.md

2 directories, 4 files
```

- `-s`, `--summary` displays the number of listed directories and files at the end.
- The starting directory itself is excluded from the count.

## 6.5 Full Root Path

```text
$ dtree -f /tmp/project
/tmp/project/
├─ bin/
└─ README.md
```

- By default, only the root's basename is displayed.
- `-f`, `--full-root` displays its full path.

# 7. Output Styles

Unicode output uses visually clear branch lines:

```text
$ dtree .
project/
├─ docs/
├─ dtree
└─ samples/
```

ASCII output is easier to use in environments prone to character encoding problems:

```text
$ dtree --ascii .
project/
+- docs/
+- dtree
+- samples/
```

Use ASCII if your terminal or log destination has difficulty with Unicode lines.

# 8. Including Hidden Files

Use `--all` to include hidden files and directories:

```text
$ dtree --all project
project/
├─ bin/
├─ README.md
├─ .gitignore
└─ .tools/
```

This is useful for inspecting layouts that include configuration files and hidden directories.

# 9. Usage Notes

`dtree` accepts only one starting directory argument.

```text
$ dtree dir1 dir2
```

Specifying multiple directories as above produces an error.

A nonexistent path or a path that is not a directory also causes the tool to display usage and exit.

```text
$ dtree not-found
```

Hidden files and directories are excluded by default. Specify `--all` when needed.
