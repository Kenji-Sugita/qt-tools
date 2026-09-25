---
genpdf:
  Format: book
  Title: qeyes User Guide
  Subtitle: Version 1.2
  Author: SRA, Inc.
---

# 1. Overview

`qeyes` is a small Qt Widgets application that keeps a pair of eyes on the desktop, following the mouse pointer.

This guide explains **how to use `qeyes` as a lightweight resident tool**, rather than simply listing its controls.

Keep the following points in mind:

- `qeyes` is a small tool intended to remain visible, rather than an ordinary decorative window.
- Controls are limited to left-dragging, right-clicking, and left-double-clicking.
- Its appearance is drawn dynamically in code rather than from images.
- Only colors and the always-on-top setting are saved; position and size are not persisted.

Think of `qeyes` as **an unobtrusive pair of desktop eyes that starts and closes quickly**, rather than a feature-rich desktop accessory.

# 2. What `qeyes` Does and Does Not Do

`qeyes` provides these simple features:

- Eyes that follow the mouse pointer
- Always-on-top display by default
- Movement by left-dragging
- Appearance settings by right-clicking
- Exit by left-double-clicking

The following features are intentionally omitted:

- Complex settings screens
- A window resizing UI
- Persistent window position or size
- Detailed shape or animation adjustments
- Major platform-specific behavior differences

`qeyes` therefore **focuses on display and minimal appearance adjustments**.

# 3. Basic Approach

Use `qeyes` as follows to keep it simple even while it remains running:

- Drag it to a convenient location.
- Right-click to adjust colors or the always-on-top setting only when needed.
- Double-click to close it when finished.
- Do not expect detailed state management.

Everyday use requires only a few short actions.

# 4. Starting the Application

Run the built binary:

```text
$ ./build/qeyes
```

For example, to build from source:

```text
$ cmake -S . -B build
$ cmake --build build
$ ./build/qeyes
```

At startup, two eyes appear in a frameless window with a transparent background.

# 5. Basic Controls to Learn First

There are only three essential controls:

```text
Left-drag: Move the window
Right-click: Open the appearance settings dialog
Left-double-click: Exit
```

These cover almost all everyday operations.

# 6. Moving the Window

Drag with the left mouse button to move the entire window.

Because `qeyes` has no title bar, drag directly on the displayed eyes instead of grabbing a window frame.

```text
Left-drag: Move the window
```

The position is not saved, so the next launch may not restore the location used when you last quit.

# 7. Changing the Appearance

Right-click to open the appearance settings dialog.

You can change these components:

- Outline: The eye outline
- Sclera: The white of the eye
- Iris: The iris
- Pupil: The pupil

Click a color preview to open the color picker. Color codes use a monospace font for easier comparison.

The same dialog lets you toggle whether the window always stays on top.

```text
Right-click: Open the appearance settings dialog
Click a preview: Choose a color
[Always on top]: Toggle always-on-top display
Reset: Restore the corresponding item to its default
OK: Save changes
Cancel: Discard changes
```

`qeyes` offers only a small set of appearance options: these four colors and the always-on-top setting.

# 8. Exiting

Double-click with the left mouse button to exit.

```text
Left-double-click: Exit
```

This makes the small resident tool quick to close.

Closing the window also exits the entire application.

# 9. Which Settings Are Saved?

The color settings and always-on-top setting are saved.

They are stored using `QSettings` and restored on the next launch.

The stored keys are:

- `colors/outline`
- `colors/sclera`
- `colors/iris`
- `colors/pupil`
- `window/alwaysOnTop`

On macOS, they are normally stored in the user's preferences area.

The following state is not saved:

- Window position
- Window size
- Shape proportions
- Tracking speed

This keeps settings management simple and the tool lightweight.

# 10. How the Display Works

`qeyes` draws its appearance dynamically in code rather than using image assets.

## 10.1 Eye Components

The left and right eyes are calculated from the window size. Each eye consists of:

- An outline
- A sclera
- An iris
- A pupil

## 10.2 Gaze Tracking

The iris and pupil centers are calculated from the center of each eye and the current mouse pointer coordinates.

Their movement is constrained to remain inside the sclera, so they never extend outside the eye while tracking.

## 10.3 Repainting

The display repaints approximately every 16 milliseconds, providing tracking at roughly 60 fps.

The gaze therefore follows pointer movements naturally without any special user action.

# 11. Window Behavior

The window has these behavioral characteristics:

- Frameless window
- Tool window
- Always on top by default
- Transparent background
- Configured not to activate at startup

Together, these make it behave like a small auxiliary desktop display rather than a conventional application window.

You can turn off always-on-top display in the appearance settings dialog.

# 12. Constraints and Notes

The current implementation has these limitations:

- No window resizing UI
- No persistent position or size
- No shape settings beyond color changes
- No detailed adjustments for multiple monitors
- Double-clicking is the intended exit operation

`qeyes` intentionally retains these constraints to stay simple. Keeping it dependable as a small resident tool takes priority over adding features.

# 13. Related Documents

See the following versioned documents and related materials:

- Documentation index: `docs/README.md`
- Sequence of prompts given to Codex while vibe-coding this application: `CODEX_PROMPT_FLOW.md`
- User guide 1.2: `docs/guides/USER_GUIDE-1.2.md`
- User guide 1.1: `docs/guides/USER_GUIDE-1.1.md`
- User guide 1.0: `docs/guides/USER_GUIDE-1.0.md`
- Release notes 1.2: `docs/releases/ReleaseNote-1.2.md`
- Release history: `docs/history/RELEASE_HISTORY.md`

# 14. Summary

`qeyes` is a small Qt application that keeps a pair of pointer-following eyes visible.

Its controls are limited to moving, changing the appearance, and exiting. The display is dynamically drawn, and only colors and the always-on-top setting are saved.

It is best used as **a resident tool that starts quickly, is easy to position, and closes immediately**, rather than as a feature-rich desktop toy.
