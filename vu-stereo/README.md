# VuStereo

`VuStereo` is a macOS stereo VU meter for system audio playback.

It displays left and right system audio levels with analog-style VU meters.
The app uses Qt 6 Widgets and macOS ScreenCaptureKit.

VuStereo is designed as a visual desktop accessory for people who enjoy audio
gear and analog VU meters. It prioritizes the look and feel of the meter over
instrument-grade measurement accuracy, and keeps features intentionally small.

## Version

The current release is recorded in [`VERSION`](VERSION). This file is the
single source for the CMake project version. Update it before configuring a
release build.

## Requirements

- macOS 13 or later
- Qt 6
- CMake 3.16 or later
- A C++17 compiler
- A local code signing identity named `vu-stereo Local Code Signing`

ScreenCaptureKit system-audio monitoring requires Screen Recording permission
on macOS. See [`SIGNING.md`](SIGNING.md) and [`USER_GUIDE.md`](USER_GUIDE.md)
for details.

## Build

```text
cmake -S . -B build
cmake --build build
open build/VuStereo.app
```

The default build signs `build/VuStereo.app` with the local code signing
identity documented in [`SIGNING.md`](SIGNING.md).

## Source Release

Create a source distribution zip:

```text
cmake --build build --target source_release
```

The output is written under `release/`:

```text
release/vu-stereo-<version>-source.zip
```

The zip contains a top-level `vu-stereo/` directory. It includes the build
files, source files, resources, translations, user guide, signing notes, and
privacy policy needed to build and review the app.
