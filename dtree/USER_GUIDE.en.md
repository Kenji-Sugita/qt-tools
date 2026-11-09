---
genpdf:
  Format: book
  Title: dtree User Guide
  Subtitle: Version 1.0.1
  Author: SRA, Inc.
  Date: 2026-11-08
---

# 1. Overview

`dtree` is a small command-line tool that displays directory contents as a tree.

It recursively walks the specified directory and displays files and subdirectories
with readable branch lines. You can switch between Unicode and ASCII-only lines,
include hidden files and directories, show the root's full path instead of its
basename, and display counts at the end. At each level, you can sort entries by
name or modification time, exclude entries by name, and limit the number of
displayed files. Optional details include entry type, modification time, size,
permissions, and owner.

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
- Normal entries at each level are sorted by name by default.
- `--sort date` sorts normal entries by modification time, newest first.
- With `--all`, hidden entries follow normal entries.
- With `--all --sort date`, normal and hidden entries are sorted separately by modification time.
- `--exclude NAME` excludes entries with that exact name at every level.
- `--max-files N` displays at most `N` files in each directory.
- Directories are not included in that limit and are always displayed.
- Directory names end with `/`.
- `-v` displays additional information about each entry.
- Hidden files and directories are excluded by default.
- `--all` includes hidden files and directories.

Without an argument, the current directory (`.`) is displayed.

# 3. Installation

Extract the distribution ZIP:

```text
$ unzip dtree-1.0.1-source.zip
$ cd dtree-1.0.1
$ chmod +x dtree
```

Run it from the current directory:

```text
$ ./dtree
```

To run it from any directory, place it on `PATH`:

```text
$ cp dtree ~/bin/
$ cp VERSION ~/bin/
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

Display the version:

```text
$ dtree --version
dtree 1.0.1
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
- Normal entries are sorted by name by default.
- `--sort date` sorts normal and hidden entries separately by modification time, newest first.
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

## 6.5 Sort Order

```text
$ dtree --sort name project
$ dtree --sort date project
```

- The default is name order, equivalent to `--sort name`.
- `--sort date` sorts entries at each level by modification time, newest first.
- Entries with the same modification time are sorted by name.
- With `--all`, normal entries still appear before hidden entries.

## 6.6 Excluding Names

```text
$ dtree --exclude build --exclude output project
project/
├─ README.md
├─ docs/
│  └─ guide.md
└─ src/
   └─ main.sh
```

- `--exclude NAME` excludes entries whose name exactly matches `NAME` at every level.
- `--exclude build` excludes `build`, but not `build-old` or `my-build`.
- You can specify `--exclude` more than once.
- Excluded directories are not traversed.
- Exclusions also apply when `--all` is specified.
- Excluded entries are not included in summary counts.
- `--max-files` is applied after exclusions.

## 6.7 Limiting Files

```text
$ dtree --max-files 1 project
project/
├─ bin/
│  ├─ run.sh
│  └─ ...
├─ lib/
│  ├─ core.sh
│  └─ ...
├─ README.md
└─ ...
```

- `--max-files N` displays at most `N` files in each directory.
- Directories do not count toward the limit and are always displayed.
- `...` indicates that files were omitted.
- `--max-files 0` displays directories and omission markers, but no file names.
- Omission markers are not included in the file count.
- Symbolic links are treated as links and count toward the file limit.

## 6.8 Full Root Path

```text
$ dtree -f /tmp/project
/tmp/project/
├─ bin/
└─ README.md
```

- By default, only the root's basename is displayed.
- `-f`, `--full-root` displays its full path.

## 6.9 Detailed Output

```text
$ dtree -v project
project/ [dir]
├─ bin/ [dir]
├─ current [link -> releases/current]
└─ README.md [file]
```

```text
$ dtree -vv project
project/ [dir, 2026-05-20 10:15]
├─ bin/ [dir, 2026-05-20 10:14]
├─ current [link -> releases/current, 16B, 2026-05-20 10:13]
└─ README.md [file, 842B, 2026-05-19 18:30]
```

```text
$ dtree -vvv project
project/ [dir, drwxr-xr-x, sugita, 2026-05-20 10:15]
├─ bin/ [dir, drwxr-xr-x, sugita, 2026-05-20 10:14]
├─ current [link -> releases/current, lrwxr-xr-x, sugita, 16B, 2026-05-20 10:13]
└─ README.md [file, -rw-r--r--, sugita, 842B, 2026-05-19 18:30]
```

- `-v`, `--verbose` displays the type of each entry.
- `-vv` adds modification times for directories, and size and modification time for files.
- `-vvv` also adds permissions and owner.
- Symbolic links are displayed as `link -> target`.
- Repeat `-v` as `-vv` or `-vvv` to increase detail.
- You can combine it with other short options, as in `-avv`.

## 6.10 Version Information

```text
$ dtree --version
dtree 1.0.1
```

- `--version` displays the version.
- The root `VERSION` file is the single source of the `dtree` version.
- `-v` controls detailed output; it does not display the version.

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
