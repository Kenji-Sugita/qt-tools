---
genpdf:
  Format: book
  Title: now User Guide
  Subtitle: Version 1.2.0
  Author: SRA, Inc.
---

# 1. Overview

`now` is a small command-line tool for quickly displaying the current date or time.

By default, it displays time as `HH:MM`. Other options include date only, time with
seconds, ISO 8601, Unix seconds, and a filename-friendly format. You can use UTC
or a specified time zone and shift the date forward or backward with `+N` or `-N`.

# 2. Requirements and Setup

`now` runs on Python 3. Download the distribution file and make it executable:

```text
$ chmod +x now
$ ./now
```

To run it from any directory, place it on `PATH`:

```text
$ cp now ~/bin/
$ chmod +x ~/bin/now
```

You can also invoke it directly with Python 3:

```text
$ python3 ./now
```

# 3. Basic Usage

Without arguments, it displays the current local time to the minute:

```text
$ now
19:30
```

Check the version:

```text
$ now --version
now 1.2.0
```

Display usage with `--help`:

```text
$ now --help
```

# 4. Output Formats

Only one output format may be specified at a time.

## 4.1 Time with Seconds

```text
$ now --time
19:30:45
```

`--time` displays hours, minutes, and seconds as `HH:MM:SS`.

## 4.2 Date

```text
$ now --date
2026-09-01
```

`--date` displays only the date, as `YYYY-MM-DD`.

## 4.3 ISO 8601

```text
$ now --iso
2026-09-01T19:30:45+09:00
```

`--iso` uses ISO 8601 format with the time zone. Combined with `--utc`, it ends in `Z`.

```text
$ now --iso --utc
2026-09-01T10:30:45Z
```

## 4.4 Unix Seconds

```text
$ now --unix
1788268245
```

`--unix` displays seconds since the Unix epoch.

## 4.5 Filename Format

```text
$ now --filename
20260901-193045
```

`--filename` uses the filename-friendly `YYYYMMDD-HHMMSS` format.

## 4.6 Custom Format

```text
$ now --format '%Y/%m/%d %H:%M'
2026/09/01 19:30
```

Use `-f FORMAT` or `--format FORMAT` to specify a Python `strftime` format.

# 5. Shifting the Date

Specify `+N` or `-N` first to display the date and time N days after or before today.

```text
$ now +1 --date
2026-09-02

$ now -7 --date
2026-08-25
```

Only one date offset may be specified.

# 6. Time Zones

Use `--utc` for UTC:

```text
$ now --time --utc
10:30:45
```

Use `--tz ZONE` to display the date and time in a region specified by its IANA time zone name:

```text
$ now --time --tz Asia/Tokyo
19:30:45

$ now --date --tz America/New_York
2026-09-01
```

`Asia/Tokyo`, `JST`, `UTC`, and `GMT` are also supported. `--utc` and `--tz` cannot be combined.

# 7. Usage Notes

Specify only one format option. Combinations such as `--date --iso` produce an error.

Pass a valid time zone name to `--tz`. Some environments may require additional time zone data.
