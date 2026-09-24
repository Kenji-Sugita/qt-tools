---
genpdf:
  Format: book
  Title: SeminarTimer User Guide
  Subtitle: Version 1.0
  Author: SRA, Inc.
---

# 1. Introduction

`SeminarTimer` is a desktop timer for managing seminars, hands-on sessions,
exercises, and breaks. It focuses on **staying visible on screen and clearly
showing the remaining time**, rather than offering a general-purpose feature set.

Main features:

- Compact, always-on-top window without a title bar
- Circular ring showing time remaining
- `5 / 10 / 20 / 30` minute presets
- Custom duration with `SET`
- `BREAK` mode
- Simple `START` / `PAUSE` / `RESET` controls
- Audible notifications at 10 seconds remaining and at the end

# 2. Requirements

This desktop application uses Qt 6 Widgets.

The application distribution targets macOS.

Building from source requires at least:

- CMake 3.21
- A C++17 compiler
- Qt 6.5

# 3. Installation

## 3.1 Installing from a DMG

Open `SeminarTimer.dmg` and copy `SeminarTimer.app` into `Applications`.

## 3.2 Extracting the Source ZIP

The distributed `.zip` contains the source needed to build the application,
not a prebuilt `.app`.

Main contents:

- `main.cpp`
- `CMakeLists.txt`
- `Info.plist.in`
- `SeminarTimer.icns`
- `assets/`
- `USER_GUIDE.md`

Extract the ZIP and build in the extracted directory:

1. Extract `seminartimer-release-YYYYMMDD.zip`.
2. Enter the extracted folder.
3. Build in an environment with CMake and Qt.
4. Launch the resulting application.

This ZIP is for source distribution and transferring build files.

## 3.3 Building from Source

Run in the project directory:

```bash
cmake -S . -B build
cmake --build build
```

Launch the generated application.

# 4. Launching

Launching `SeminarTimer.app` displays the timer near the upper-right corner of the screen.

The window:

- Stays on top
- Has no title bar
- Can be dragged to any position

# 5. Understanding the Display

The timer shows:

- Top title: `SEMINAR TIMER`
- Center: Time remaining
- Below center: Status
- Outer ring: Countdown progress
- Bottom buttons: `START` or `PAUSE`, and `RESET`
- Top chips: Duration presets, `SET`, and `BREAK`

Status changes according to the timer state:

- `READY`: Stopped
- `IN PRACTICE`: Running in normal mode
- `ON BREAK`: Running in BREAK mode
- `WRAP UP`: 10 seconds or less remaining

A `BREAK MODE` badge appears when BREAK mode is enabled.

# 6. Basic Controls

## 6.1 Start and Pause

Press `START` to begin counting down.

While running, the same button becomes `PAUSE`; press it to pause.

## 6.2 Reset

`RESET` restores the currently selected duration:

- The selected preset, if using a preset
- The custom duration, if set with `SET`

## 6.3 Moving the Window

Drag an area other than a button or chip to move the window.

# 7. Setting the Duration

## 7.1 Presets

The chips at the top offer:

- `5 min`
- `10 min`
- `20 min`
- `30 min`

Selecting a preset makes it the new base duration.

## 7.2 Custom Duration

Press `SET` to open the duration dialog.

1. Change the number of minutes with `-` or `+`.
2. Press `OK` to confirm.

The range is **1 to 180 minutes**.

After confirmation, the value becomes the base duration and is also restored by `RESET`.

`Cancel` closes the dialog without changes.

# 8. BREAK Mode

Press `BREAK` to toggle BREAK mode.

This changes **the display mode**, not the timer duration.

The main changes are:

- A color scheme suited to breaks
- `ON BREAK` status while running
- A `BREAK MODE` badge

# 9. Notifications and Display Changes

The display and sounds change as time runs out.

## 9.1 10 Seconds or Less Remaining

- Status becomes `WRAP UP`.
- Colors shift toward a warning palette.
- The display starts flashing.
- A notification sounds once.

## 9.2 Time Expired

The timer stops automatically at zero.

The notification sounds three times.

# 10. Keyboard Controls

- `Space`: Start / pause
- `R`: Reset
- `T`: Open the duration dialog
- `B`: Toggle BREAK mode
- `0`: Select 5 minutes
- `1`: Select 10 minutes
- `2`: Select 20 minutes
- `3`: Select 30 minutes
- `Esc`: Close the dialog, or exit the application if no dialog is open

# 11. Duration Dialog

The dialog overlays the center of the main display.

- Change minutes with `-` / `+`, not direct text entry.
- The value is right-aligned.
- `OK` applies it.
- `Cancel` discards it.
- Clicking outside closes the dialog.
- `Esc` also closes it.

# 12. Exiting

Either:

- Press `Esc`. If the duration dialog is open, the first press closes that dialog.
- Use the standard macOS application quit operation.

# 13. Example Uses

- Sharing exercise time in an online seminar
- Screen-sharing the countdown during a hands-on session
- Showing attendees how long a break lasts
- Personal time management for instructors or facilitators

# 14. Typical Workflow

1. Launch the application.
2. Select a preset or set a duration with `SET`.
3. Toggle `BREAK` if needed.
4. Press `START`.
5. Use `PAUSE` or `RESET` as needed.
6. Close the application when time expires or the activity ends.

# 15. Additional Notes

The application is designed for simple operation:

- No fine-grained settings in seconds
- No simultaneous management of multiple timers
- BREAK mode is a display mode

Use it as a lightweight, easy-to-read timer that is straightforward to operate during a seminar.
