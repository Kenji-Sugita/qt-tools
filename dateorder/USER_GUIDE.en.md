# dateorder

`dateorder` is a small utility that checks whether files supplied on the command line are ordered by modification time.

The executable is `younger`; `older` is a symbolic link to it. The invocation name determines the comparison direction.

## Commands

- `younger`: Check whether the files are ordered newest first.
- `older`: Check whether the files are ordered oldest first.

Comparisons use each file's modification time (`mtime`).

## Usage

```text
younger FILE FILE...
older   FILE FILE...
```

- `younger a b c` succeeds if `a.mtime >= b.mtime >= c.mtime`.
- `older a b c` succeeds if `a.mtime <= b.mtime <= c.mtime`.

Equal modification times are allowed.

## Exit Codes

- `0`: The requested order is satisfied.
- `1`: The requested order is not satisfied.
- `2`: Usage or runtime error.

Examples resulting in `2`:

- Fewer than two arguments
- A nonexistent file
- A failed `stat` call

## Output

No output is produced on success.

Example order violation:

```text
younger: order violation between 'old.txt' and 'new.txt'
```

Example missing file:

```text
older: cannot stat 'missing.txt'
```

## Examples

```bash
# Check newest-first order
younger newest.log middle.log oldest.log

# Check oldest-first order
older  20240101.txt 20240201.txt 20240301.txt

# Use in a shell conditional
if younger *.log; then
  echo OK
else
  echo NG
fi
```

## Building

```bash
cmake -S . -B build
cmake --build build
```

The build creates `build/younger` and a symbolic link at `build/older`.

## Installation

You can run the built executables directly without installing them system-wide:

```bash
./build/younger FILE FILE...
./build/older   FILE FILE...
```

To install on `PATH`, copy `younger` and create the `older` symbolic link.

Example:

```bash
install -m 0755 build/younger /usr/local/bin/younger
ln -sf younger /usr/local/bin/older
```

Use `sudo` if writing to `/usr/local/bin` requires administrator privileges.

Example post-installation check:

```bash
younger --help
older --help
```

The current implementation does not have a `--help` option. These commands
check that usage information is displayed and return exit code `2`.
