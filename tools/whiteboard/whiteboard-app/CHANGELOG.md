# Changelog

## 1.3.0 - 2026-11-09

### Added

- Added editable drawing data export for all pages or the current page using the versioned `.whiteboard` JSON format.
- Added validated import of `.whiteboard` and legacy autosave JSON as pages after the current page, with remapped element/connector IDs and one-step Undo/Redo.
- Added persistent save-failure status, explicit save retry, protected startup after restore failure, and close-time choices that preserve unsaved edits.
- Added persistence information to MCP state and mutation results so applied edits are distinguished from failed saves.

### Fixed

- Verify complete writes and atomic commits for normal document saves, preserving the existing file on failure.
- Prevent failed restoration from overwriting the original with an empty document; reject malformed structures and documents above 20 pages while retaining supported legacy normalization.
- Prevent drawing-data and image exports from replacing the normal save file through the same path or a file alias.

## 1.2.0 - 2026-09-18

### Changed

- Automatically approve MCP `whiteboard/diagram/apply` requests in all modes while retaining confirmation for other mutation tools and image file saves.

## 1.1.0 - 2026-09-06

### Added

- Added MCP tools to list page elements and atomically update or delete current-page elements by persistent ID.
- Added document revision conflict detection, combined MCP element changes, and temporary-to-persistent ID mappings.
- Added persistent IDs for every drawing element and document-level ID allocation.
- Added connector lines whose endpoints attach to shapes and follow move, resize, rotation, and scaling changes.
- Added connector endpoint reconnection by dragging a line endpoint onto another shape.
- Added image import from PNG, JPEG, BMP, and SVG files.
- Added direct image paste from the OS clipboard and image file drag-and-drop onto the canvas.
- Added embedded image elements that can be selected, moved, resized, rotated, copied, pasted, grouped, deleted, saved, and restored.

### Changed

- Updated the operation cheatsheet for version 1.1.0 with rotation, connector, image import/export, and Undo/Redo guidance.
- Element IDs are no longer reused when an element-creating operation is undone and replaced by a new edit.
- Legacy saved files without element IDs are normalized on load by assigning unique IDs without changing geometry or appearance.
- Copy/paste, MCP diagram apply, and SVG export now use fresh persistent element IDs for newly created elements.
- Copy/paste and MCP diagram apply now remap connector endpoint references to newly allocated element IDs.
- SVG import is rasterized to embedded PNG image data.
- SVG export now embeds imported image elements as Base64 data URI `<image>` elements while keeping non-image elements as vector output.

## 1.0.0 - 2026-09-02

### Added

- Added the first release version marker for WhiteboardApp.
- Added release notes and CMake version validation based on the `VERSION` file.

### Changed

- Use `VERSION` as the source for the CMake project version, application version, and MCP server information.
- Embedded the WhiteboardApp-specific MCP JSON-RPC / Streamable HTTP implementation in the application source tree.
- Removed the build-time and runtime dependency on an external qtmcpserver checkout.
