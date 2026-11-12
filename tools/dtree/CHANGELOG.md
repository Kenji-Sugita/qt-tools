# Changelog

## 1.0.1 - 2026-11-08

### Fixed

- Corrected the archive name and extracted directory in the installation guide.
- Synchronized the Markdown guide, PDF, and source distribution.
- Kept the 1.0.0 command implementation and options unchanged.

## 1.0.0 - 2026-08-20

### Added

- Added `--version`, backed by the root `VERSION` file.
- Added verbose display levels with `-v`, `-vv`, and `-vvv`.
- Added symbolic link display as `link -> target`.
- Added `--max-files N` to limit displayed files per directory.
- Added `...` when files are omitted by `--max-files`.
- Added `--exclude NAME` for repeatable exact-name exclusions.

### Changed

- The root `VERSION` file is the single source for the displayed `dtree`
  version.
