# tl User Guide

## Overview

`tl` automatically detects Japanese or English input and translates it into the other language.

- Japanese input is translated into English.
- English input is translated into Japanese.
- The current translation engine is Google Cloud Translation.
- Only `ja` and `en` are supported.

## Prerequisites

Before use, ensure that:

- A Qt 6 development environment is available.
- Google Cloud Translation API is enabled.
- A Google Cloud billing account is configured.
- You have a service account JSON key.
- `GOOGLE_APPLICATION_CREDENTIALS` is set.

See GOOGLE_FREE_SETUP.md for Google Cloud setup.

Example environment variable:

```bash
export GOOGLE_APPLICATION_CREDENTIALS="$HOME/.config/tl/google-translation-service-account.json"
```

## Building

Run from the repository root:

```bash
cmake -S . -B build
cmake --build build
```

Generated executable:

```bash
./build/tl
```

## Basic Usage

Usage:

```bash
tl [text]
tl --file <path>
tl --help
tl --version
```

### 1. Translating a Positional Argument

The Japanese example means “I have a meeting today.”

```bash
./build/tl "今日は会議があります。"
```

```bash
./build/tl "Please check this file."
```

### 2. Translating a File

```bash
./build/tl --file input.txt
```

`--file` reads the entire file and translates it as one text.

### 3. Translating Standard Input

The first example means “This issue has not been resolved yet.” The interactive
example says “I have a meeting today. I will send the materials later.”

```bash
echo "この問題はまだ解決していません。" | ./build/tl
```

```bash
cat memo.txt | ./build/tl
```

```bash
./build/tl
今日は会議があります。
資料はあとで送ります。
^D
```

## Input Rules

Only one input source is used, in this priority order:

1. Positional argument `text`
2. `--file <path>`
3. Standard input

The following produce errors:

- Supplying two or more positional arguments
- Combining a positional argument with `--file`
- Supplying standard input together with `--file`
- Providing empty input

## Output

- On success, only the translation is written to standard output.
- No explanatory text is added.
- Errors are written to standard error as `tl: ...`.

## Common Options

- `--help`: Display help.
- `--version`: Display the version.
- `--file <path>`: Use a file as input.

## Notes

- Detection of any language other than `ja` or `en` produces an error.
- Multiline input is supported, but output line breaks are not guaranteed to be preserved.
- Trailing line breaks are removed before translation.
- The tool cannot run if the credentials file is unreadable.

## Verification

Display help:

```bash
./build/tl --help
```

Display the version:

```bash
./build/tl --version
```

Basic tests:

```bash
ctest --test-dir build --output-on-failure
```

These tests primarily check CLI argument handling and empty-input errors.
They do not test live communication with Google Cloud.

## Troubleshooting

`tl: GOOGLE_APPLICATION_CREDENTIALS is not set`

- Set the `GOOGLE_APPLICATION_CREDENTIALS` environment variable.

`tl: cannot read credentials file '...'`

- Check the path to the JSON key file.
- Check that the user running the command has read permission.

`tl: openssl executable not found at /opt/local/bin/openssl`

- Install `openssl` at `/opt/local/bin/openssl`.

`tl: API request failed`

- Check that Google Cloud Translation API is enabled.
- Check that the credentials' project can use Translation API.
- Check the network connection.

`tl: detected language '...' is not supported`

- Check that the input can be recognized as Japanese or English.

## Related Documents

- Specification: SPEC.md
- Google Cloud setup: GOOGLE_FREE_SETUP.md
