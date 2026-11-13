# Publication of Ten Reviewed Resources (2026-11-13)

Canonical sources: `~/Sites`, commit `ee33ffd`. The user requested publication to this repository after the source review and revision. Eight documentation groups and two tools are included; the internal Qt-business AI workbook is excluded. Existing Whiteboard workflow history and all other projects are preserved.

## Scope and provenance

`manifest.json` records 35 unchanged canonical files and 39 files extracted unchanged from the two tool archives, with 70 distinct destinations. The three distribution ZIPs pass CRC checks. Sources, Japanese PDFs and ZIPs are not edited or regenerated here. Codex-at-work contains only the main guide, Windows supplement and introductory slides (six files), excluding the separate hearing PDF and codex.pdf.

Japanese documents have English navigation READMEs. The two tools also have English user guides. The fileservermcp guide preserves all 82 command/JSON/diagram blocks, with translated Mermaid labels. The original general workbook README remains unchanged. This is not a full English translation of the eight documentation groups.

The copied fileservermcp CMake file already follows the canonical CMake guidance for Qt console/multiple-target projects; it is unchanged from the archive. No dependency or build feature was added.

## Validation

- `check_publication.py`: canonical bytes and hashes, archive entries, CRCs, local Markdown references and balanced code fences, and bilingual fileservermcp code-block correspondence. Results: `publication-checks.json`.
- Configure: `cmake -S tools/fileservermcp -B <temporary-build> -DCMAKE_PREFIX_PATH=<Qt-6.12.0-macos>`; build with `cmake --build <temporary-build> -j 4`. The build was separate from the checkout. Build output: `fileserver-build.log`.
- `check_fileserver.py --build <temporary-build> --report <result.json>`: 13 checks with dummy files and an ephemeral loopback port. Includes initialization, tool listing, listing restrictions, exact downloaded bytes, rejection cases and REPL. The server is terminated after verification. Results: `fileserver-checks.json`.
- `node --check tools/warikan/src/app.js` and `node verification/next10-20261113/check_warikan.cjs`: seven logic checks plus a static persistence-API check. Results: `warikan-checks.json`.
- Git whitespace validation applies to added navigation, English guides, indexes, verification scripts and maintenance records. Whitespace in exact canonical/archive copies is preserved and checked by byte/hash comparison instead of rewriting the originals.

The runtime environment was macOS arm64 / Qt 6.12.0. Current Qt Creator connectivity, browser GUI interactions, other operating systems, beginner usability and production effectiveness are not newly tested. Qt Creator examples retain the 19.0.0/legacy HTTP+SSE qualification; the tutorial server remains non-production. A link annotation or Git copy is not a test of an external service.
