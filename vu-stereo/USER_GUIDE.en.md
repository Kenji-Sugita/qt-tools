---
genpdf:
  Format: book
  Title: VuStereo User Guide
  Subtitle: Version 1.0
  Author: SRA, Inc.
---

# 1. Overview

`VuStereo` is a desktop stereo VU meter that displays the left and right channels of system audio playing on macOS.

It places an analog VU meter-style display on your desktop while you listen to music in applications such as Music.

This application is not a precision audio measurement instrument. It is an accessory for people who enjoy audio equipment and VU meters and want to watch the needles move while listening to music.

It prioritizes natural, pleasing needle motion and appearance over measurement accuracy. Its features are limited to what is needed for everyday use.

Main features:

- Two VU meters for the left and right channels
- Monitoring of macOS system audio levels
- A frameless window with a transparent background
- Window movement by dragging
- Saved window position
- An always-on-top toggle
- A choice of arc or linear scales
- Peak hold display
- Adjustments for input level, needle width, and pivot display
- Japanese and English display languages

`VuStereo` does not change the volume, record audio, or save it. It is a small resident tool for enjoying a visual representation of the audio currently playing.

# 2. System Requirements

This macOS application uses Qt 6 Widgets.

It captures audio through macOS ScreenCaptureKit, so running it requires:

- macOS 13 or later
- Screen Recording permission
- Stable code signing

Building from source requires at least:

- CMake
- A C++17-capable compiler
- Qt 6
- Qt Widgets
- Qt Svg
- Qt LinguistTools

# 3. Installation

To install from the distributed `VuStereo.dmg`:

1. Double-click `VuStereo.dmg` to open it.
2. Drag the displayed `VuStereo.app` to the `Applications` folder to copy it.
3. Once copying finishes, eject the `VuStereo` disk image from the Finder sidebar or desktop.
4. Launch `VuStereo.app` in the `Applications` folder from Finder.

On the first launch, grant macOS Screen Recording permission. After granting permission, quit `VuStereo` completely and restart it.

Although the application can run from `VuStereo.dmg`, normally you should copy it to the `Applications` folder first.

# 4. Starting the Application

For a prebuilt application, launch `VuStereo.app` from Finder.

An example build from source is:

```bash
cmake -S . -B build
cmake --build build
open build/VuStereo.app
```

Because the application depends on Screen Recording permission, the standard approach is to launch it as an `.app` from Finder, including for development builds.

# 5. First Launch and Screen Recording Permission

macOS may request Screen Recording permission on the first launch.

To grant permission:

1. Launch `VuStereo.app` from Finder.
2. If a permission dialog appears, open System Settings.
3. Open `Screen Recording` under `Privacy & Security`.
4. Turn on `VuStereo` in the list.
5. Quit `VuStereo` completely.
6. Relaunch `VuStereo.app` from Finder.

Audio monitoring may not become active until the application is restarted after permission is granted.

See `SIGNING.md` for code signing and distribution notes.

# 6. Audio Input Method

`VuStereo` monitors system audio through macOS ScreenCaptureKit rather than directly opening a microphone or audio interface input.

It receives audio playing in applications such as Music from macOS and calculates the left and right channel levels in dBFS. Only the calculated values are passed to the VU meter display.

Key points:

- There is no audio input device selector.
- There is no option to monitor only a selected application.
- System audio samples are used only to calculate levels.
- Audio data is not recorded, saved, or transmitted.

macOS requires Screen Recording permission even for audio capture through ScreenCaptureKit. You may therefore be asked to grant permission on first launch or after changing the code signature.

The same explanation is available from this application menu:

```text
VuStereo > Audio Input and Privacy...
```

# 7. Reading the Display

At startup, two VU meters appear:

- `L`: Left channel
- `R`: Right channel
- Black needle: Current audio level
- Red needle: Peak hold display
- Numbers at the bottom: Current left and right levels

The numbers at the bottom are in dBFS, for example:

```text
L -30.9 dBFS   R -30.9 dBFS
```

Values decrease toward silence and approach 0 dBFS toward the maximum level.

# 8. Basic Controls

Everyday operations are:

```text
Drag: Move the window
Application menu > Settings...: Open settings
Application menu > Audio Input and Privacy...: Show the input method and permission explanation
Application menu > Restart audio monitor: Restart audio monitoring
Standard macOS operation: Quit the application
```

The window has no title bar; drag its base directly to move it.

The window position is saved and restored on the next launch.

# 9. Settings

Open settings from the macOS application menu:

```text
VuStereo > Settings...
```

Changes take effect immediately and are saved using `QSettings`.

## 9.1 Input Level

Adjusts the input level in dB.

The range is `-24 dB` to `+24 dB`.

Lower it if the needles move too much during music playback, or raise it if they move too little. Adjust it to suit your playback environment.

## 9.2 Needle Width

Changes the width of the needles.

The range is `1` to `30`, with a default of `6`.

## 9.3 Scale

Switches the meter image and needle motion:

- `Arc`: A display with an arc-shaped scale
- `Linear`: A display with a straight scale

In `Arc` mode, needles rotate around a pivot. In `Linear` mode, vertical needles move left and right.

## 9.4 Language

Changes the display language:

- `System`: Follow the macOS language setting
- `English`: English
- `Japanese`: Japanese

The running interface updates when the setting changes.

## 9.5 Show Black Pivot

Toggles the black dot at the needle pivot in arc mode.

Turning it off hides the dot without changing needle motion.

## 9.6 Lower Needle Pivot

Toggles a lower needle pivot in arc mode.

This is on by default. When enabled, the pivot is placed below the meter image for a more realistic appearance, and the lower part of the needle is drawn within the yellow area.

Turning it off places the pivot near the black dot in the background image.

## 9.7 Show Peak Hold

Toggles the peak hold needles.

This is off by default. When enabled, a red peak hold needle appears in addition to each current-level needle.

## 9.8 Always on Top

Toggles whether `VuStereo` stays in front of other windows.

When enabled, the meters are less likely to be hidden while listening to music or working. When disabled, other windows can cover it like a normal window.

# 10. Meter Behavior

Internally, audio levels are handled in dBFS.

The expected range passed to `setValue()` is `-60 dBFS` at the silent end to `0 dBFS` at the maximum end. Values outside this range are clipped for display.

Conversion to the VU scale is an approximation intended to behave as follows:

- Around `-60 dBFS`: Far left
- Around `-18 dBFS`: The `0` mark
- `-9 dBFS` and above: Near the `+3` mark

At high levels, the needle stops at `+3`, making the display appear clipped. Adjust `Input level` so the motion looks natural in your playback environment.

# 11. Arc and Linear Scales

`VuStereo` has two background images:

- `images/vu-background.svg`
- `images/vu-linear-background.svg`

`Arc` uses `vu-background.svg`. The scale is interpolated to match the background SVG, and the needles rotate around their pivots.

`Linear` uses `vu-linear-background.svg`. Because the scale is straight, the needles remain vertical while moving left and right.

Both show the same audio levels, but differ in appearance and needle motion.

# 12. Saving Settings

Settings are saved with `QSettings`.

The main stored keys are:

- `audio/inputLevelDb`
- `meter/needleWidth`
- `meter/pivotVisible`
- `meter/needlePivotLowered`
- `meter/peakHoldEnabled`
- `meter/scaleStyle`
- `window/alwaysOnTop`
- `window/position`
- `ui/language`

These settings are restored on the next launch.

The window position is also saved, so the next launch restores its last location.

# 13. Privacy Policy

`VuStereo` does not record, save, or transmit audio.

Audio samples received from ScreenCaptureKit are used only to calculate the left and right channel levels in dBFS and are not retained afterward.

See `PRIVACY.md` for details.

# 14. Signing and Distribution

macOS Screen Recording permission is tied to the application's bundle identifier and code signature.

For this reason, even development builds of `VuStereo` are signed with a stable local code signing certificate.

See `SIGNING.md` for specific procedures involving signing, `macdeployqt`, `.zip`, and `.dmg`.

Key points:

- Sign the `.app` after running `macdeployqt`.
- Sign the `.app` before packaging it as `.zip` or `.dmg`.
- Re-sign the `.app` whenever its contents change.
- Grant Screen Recording permission separately on each other Mac.

# 15. Troubleshooting

## 15.1 The Needles Do Not Move

Check that:

- Audio is actually playing in Music or another application.
- `VuStereo` is enabled in macOS Screen Recording permissions.
- You quit `VuStereo` completely and restarted it after changing permissions.
- Running `Restart audio monitor` makes a difference.
- The `.app` is correctly signed.

## 15.2 Granting Screen Recording Permission Has No Effect

Screen Recording permission depends on the bundle identifier and code signature, not just the application name.

If permission stops working after a rebuild, follow `SIGNING.md` to check the signing state.

## 15.3 The Application Cannot Be Opened

If Finder says the application may be damaged or incomplete, you may be launching an unfinished build or an `.app` with a broken signature.

Perform a clean build and launch a correctly signed `.app`.

## 15.4 The Display Language Does Not Change

Check `Language` in settings.

`System` follows the macOS language setting. To switch immediately, select `English` or `Japanese`.

# 16. Summary

`VuStereo` is a small application for watching music playing on macOS through a pair of left and right VU meters.

It is intended as a desktop accessory for enjoying the appearance of audio equipment and moving needles, rather than as a precise measuring instrument.

Start by granting Screen Recording permission and adjusting `Input level` while music plays. Then choose your preferred scale, needle width, pivot display, peak hold, and display language.
