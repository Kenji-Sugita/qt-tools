qtcling Source Tree
===================

This directory contains the source for qtcling, a Cling C++ REPL environment
for working with Qt interactively. The current user release targets the
terminal-based `qtcling` interactive environment. See `USER_GUIDE.md` for
installation and usage instructions.

Top-level Files
---------------

USER_GUIDE.md   User guide for qtcling 1.2.0.
USER_GUIDE.pdf  PDF user guide for qtcling 1.2.0.
build.sh        Fetches Cling/LLVM, applies patches, configures, and builds on macOS/Linux.
install.sh      Installs macOS/Linux build results and the qtcling launcher.
install-win.ps1 Installs the Windows 11 ARM64 version in the user's local application data.
build-all.sh    Builds a prepared Cling/LLVM source tree.
build-win.sh    Builds and installs the native Windows 11 ARM64 version with -j 2.
build-libedit-win.ps1
                Builds and installs portable libedit for Windows 11 ARM64.
QTCLING_VERSION Holds the qtcling release version.
NEXT.md         Short handoff notes for the next development session.
memo.md         Development history and troubleshooting notes.

Top-level Directories
---------------------

bin/            User launch scripts and supporting tools.
docs/           Design notes, release plans, samples, and development documentation.
examples/       C++ and Qt sample programs for Cling/qtcling.
misc/           Experimental material, platform-specific notes, and work notes.
patch/          Required and reference patches for Cling, LLVM, and wrappers.
qtcling-gui/    Experimental GUI frontend; outside the scope of the 1.2.0 user release.
release/        Source ZIP distributions. ZIP staging uses a temporary directory.
sessions/       Development session records organized by date.
src/            qtcling startup code and callbacks loaded by the REPL.
windows/        CMake project for the Qt callback DLL for Windows 11 ARM64.

Local Configuration and Management Directories
---------------------------------------------

.git/           Git repository metadata.
.agents/         Local agent workspace management information.
.codex/          Local Codex settings and management information.

Notes
-----

Building may create large local directories such as `build/`, `cling/`, and
`llvm-project/`. They are omitted from the list above when absent. Do not delete
them without checking whether they are needed.
