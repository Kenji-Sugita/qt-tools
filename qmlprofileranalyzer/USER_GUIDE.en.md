---
genpdf:
  format: book
  title: qmlprofileranalyzer User Guide
  subtitle: Current version
  author: SRA, Inc.
  page_numbers: true
---

# 1. Overview

`qmlprofileranalyzer` is a command-line tool for reading Qt QML Profiler traces
and inspecting their contents as text.

It currently supports these uses:

- Reading `.qtd` / XML traces
- Reading `.qzt` / binary traces
- Inspecting QML Profiler summaries, types, events, and notes
- Inspecting range hierarchies and `selfTime`
- Examining hotspots, outliers, bursts, possible image-related effects, and `Compiling` caveats
- Producing supporting information for comparison with Qt Creator
- Extracting evidence for investigations and evaluation documents

Supported input formats:

- `.qtd`
- `.qzt`

CLI help and usage show the input as `<trace.qtd|trace.qzt>`. `.qtd` is XML;
`.qzt` is Qt Creator's binary format.

Analysis output is currently text only. JSON and CSV are not available.

# 2. Basic Concepts

The tool reads a trace and displays the requested view. Commands other than
`compare` take one input file; `compare` takes two.

- `summary`: Overall statistics
- `ranges`: Range hierarchies, `selfTime`, `gapWarning`, and `focus`
- `hotspots`: Aggregated expensive locations based on `selfTime`
- `compare`: Compare the `hotspots` of two traces by `groupKey`
- `convert`: Convert between `.qtd` and `.qzt`
- `types`: List event types
- `events`: Display raw events unchanged
- `events-aggregated`: Raw events plus aggregate rows for comparison with Qt Creator
- `notes`: Display notes

A single argument is treated as `summary`:

```text
$ qmlprofileranalyzer trace.qzt
```

This is equivalent to:

```text
$ qmlprofileranalyzer summary trace.qzt
```

# 3. Building

Example build with Qt 6.11.0:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos
cmake --build build
ctest --test-dir build --output-on-failure
```

Some environments may show Qt license service warnings. If needed when running
tests, add `QTFRAMEWORK_BYPASS_LICENSE_CHECK=1`:

```bash
QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 ctest --test-dir build --output-on-failure
```

Generated executable:

```text
./build/qmlprofileranalyzer
```

# 4. Running

Typical invocation:

```text
$ ./build/qmlprofileranalyzer summary samples/minimal.qtd
```

Minimal invocation:

```text
$ ./build/qmlprofileranalyzer samples/minimal.qtd
```

Display help:

```text
$ ./build/qmlprofileranalyzer --help
```

# 5. Quick Start for First-Time Users

Start with these three commands.

## 5.1 Get an Overview

```text
$ ./build/qmlprofileranalyzer summary trace.qzt
```

Look at:

- Overall trace size
- `ranges`
- `pointEvents`
- `longestRange`
- Whether `Compiling` is present

Notes:

- `summary`'s `totalDuration` includes parent–child overlap.
- Do not judge the cost of `Creating` / `Binding` from `summary` alone.

## 5.2 Find Expensive Locations

```text
$ ./build/qmlprofileranalyzer hotspots trace.qzt
```

Look at:

- `selfTotal`
- `inclusiveTotal`
- `calls`
- `medianSelf`
- `maxSelf`
- `outlier`
- `burst`
- `image`
- `compilingNote`

`hotspots` is the main command for narrowing down expensive locations.

## 5.3 Investigate Details

```text
$ ./build/qmlprofileranalyzer ranges trace.qzt
```

Look at:

- `selfTime`
- `childTime`
- `gapTime`
- `gapWarning`
- `focus`
- `depth`
- `parent`

`ranges` is the main command for inspecting parent–child relationships.

For a first look, `summary -> hotspots -> ranges` is enough.
Use `types`, `events`, and `events-aggregated` when needed.

# 6. First Examples

## 6.1 Read the Minimal `.qtd`

```text
$ ./build/qmlprofileranalyzer summary samples/minimal.qtd
```

Available information includes:

- `traceStart`
- `traceEnd`
- `traceDuration`
- `eventTypes`
- `events`
- `ranges`
- `pointEvents`
- `notes`
- `longestRange`

## 6.2 Inspect the Type List

```text
$ ./build/qmlprofileranalyzer types samples/minimal.qtd
```

Available information includes:

- Type name
- Display name
- File / line / column
- Detail information
- Details

## 6.3 Inspect Ranges

```text
$ ./build/qmlprofileranalyzer ranges tests/data/samegame.qzt
```

Available information includes:

- `duration`
- `selfTime`
- `childTime`
- `gapTime`
- `gapWarning`
- `focus`
- `depth`
- `parent`

## 6.4 Inspect Hotspots

```text
$ ./build/qmlprofileranalyzer hotspots tests/data/samegame.qzt
```

Available information includes:

- `selfTotal`
- `inclusiveTotal`
- `groupKey`
- `indexes`
- `calls`
- `minSelf`
- `medianSelf`
- `maxSelf`
- `outlier`
- `burst`
- `image`
- `compilingNote`

## 6.5 Inspect Raw Events

```text
$ ./build/qmlprofileranalyzer events samples/minimal.qtd
```

`events` is exclusively for raw events. Use it to inspect the original event sequence.

## 6.6 Inspect Events with Aggregates

```text
$ ./build/qmlprofileranalyzer events-aggregated tests/data/samegame.qzt
```

`events-aggregated` adds a `# aggregates` section at the beginning.

It currently provides supplementary aggregates for:

- `PixmapCache`
- `SceneGraph`
- `MemoryAllocation`
- `DebugMessage`

It also adds these fields to assist comparison with Qt Creator:

- `qtCreatorCategory`
- `qtCreatorLabel`
- `qtCreatorType`
- `qtCreatorThread`
- `qtCreatorStages`

# 7. Command Reference

## 7.1 `summary`

```text
$ ./build/qmlprofileranalyzer summary trace.qzt
```

Displays overall statistics.

Useful for:

- Checking trace size
- Finding the longest range
- Getting an initial idea of where to investigate
- Checking whether `Compiling` is present

Notes:

- `totalDuration` includes parent–child overlap.
- Use `hotspots` and `ranges` for detailed analysis.

## 7.2 `ranges`

```text
$ ./build/qmlprofileranalyzer ranges trace.qzt
```

Displays range hierarchies and `selfTime`.

Useful for:

- Checking parent–child overlap
- Finding genuinely expensive leaf ranges
- Checking `gapWarning`
- Inspecting `focus` candidates

Main fields:

- `duration`: Total range duration
- `selfTime`: Time excluding child ranges
- `childTime`: Time covered by child ranges
- `gapTime`: Currently the same value as `selfTime`
- `gapWarning`: Candidate time not readily explained by child QML ranges
- `focus`: Candidate whose self time stands out relative to its descendants

## 7.3 `hotspots`

```text
$ ./build/qmlprofileranalyzer hotspots trace.qzt
```

Aggregates expensive locations.

Useful for:

- Narrowing down expensive locations based on `selfTime`
- Finding outliers
- Finding possible bursts
- Finding possible image-related effects
- Checking `Compiling` caveats

Main fields:

- `calls`: Call count
- `groupKey`: Stable aggregation key for comparing traces
- `indexes`: Event indexes merged into this hotspot
- `selfTotal`: Total self time
- `inclusiveTotal`: Total time including parent–child overlap
- `medianSelf`: Median self time
- `maxSelf`: Maximum self time
- `outlier`: Possible outlier
- `burst`: Candidate concentrated in a short period
- `image`: Candidate overlapping image loading
- `compilingNote`: Caveat for interpreting `Compiling`

## 7.4 `compare`

```text
$ ./build/qmlprofileranalyzer compare before.qzt after.qzt
```

Matches two traces' `hotspots` by `groupKey` and displays differences.

Useful for:

- Checking variation across repeated measurements of the same operation
- Finding locations that became more or less expensive after a change
- Comparing traces with different `eventIndex` values

Main fields:

- `status`: `added`, `removed`, `changed`, or `unchanged`
- `groupKey`: Hotspot aggregation key used for comparison
- `beforeSelfTotal` / `afterSelfTotal`: Total self time before and after
- `selfDelta` / `selfDeltaPct`: Difference in total self time
- `beforeCalls` / `afterCalls`: Call counts
- `callsDelta`: Call-count difference
- `medianSelfDelta`: Difference in median self time
- `maxSelfDelta`: Difference in maximum self time

`compare` is limited to hotspot comparison. After finding a `groupKey` with a
large difference, inspect each trace's `ranges` for hierarchy and `gapWarning` details.

## 7.5 `types`

```text
$ ./build/qmlprofileranalyzer types trace.qzt
```

Displays type information.

Useful for:

- Finding recorded code locations
- Inspecting `Creating`, `Binding`, `Javascript`, `Compiling`, and other types
- Mapping `eventIndex` to file / line

## 7.6 `events`

```text
$ ./build/qmlprofileranalyzer events trace.qzt
```

Displays raw events.

Useful for:

- Inspecting raw data
- Following the event sequence before aggregation
- Verifying the supplementary aggregates in `events-aggregated`

## 7.7 `events-aggregated`

```text
$ ./build/qmlprofileranalyzer events-aggregated trace.qzt
```

Displays aggregate rows together with raw events.

Useful for:

- Comparison with Qt Creator
- Inspecting `PixmapCache` load times
- Observing `SceneGraph` frames
- Inspecting `MemoryAllocation` summaries by type
- Inspecting `DebugMessage` entries by level

## 7.8 `notes`

```text
$ ./build/qmlprofileranalyzer notes trace.qtd
```

Displays notes.

Useful for:

- Inspecting `noteData`
- Reviewing analysis notes

# 8. Differences Between `.qtd` and `.qzt`

The tool reads both, but their roles differ slightly:

- `.qtd`: XML; close to CLI `qmlprofiler` output
- `.qzt`: Binary; Qt Creator's default save format

`.qzt` is the more natural choice for direct comparison with Qt Creator.

Use `convert` to turn `.qzt` into readable `.qtd` XML, or convert `.qtd` to the
binary `.qzt` format for convenient storage and sharing.

```bash
./build/qmlprofileranalyzer convert trace.qzt trace.qtd
./build/qmlprofileranalyzer convert trace.qtd trace.qzt
```

`convert` determines direction from the extensions. Supported conversions are
`.qzt -> .qtd` and `.qtd -> .qzt`.

The `.qzt` format does not directly store `.qtd`'s `eventData totalTime`.
After `.qtd -> .qzt` conversion, `summary` therefore treats `totalTime` as `traceEnd - traceStart`.

# 9. Analysis Workflow

The following order is usually helpful:

1. `summary`
2. `hotspots`
3. `ranges`
4. `types`
5. `events-aggregated`, if needed
6. `events`, if needed

Notes:

- `summary` is for initial inspection.
- Detailed analysis centers on `hotspots` and `ranges`.
- `Creating` / `Binding` can be misleading without considering their hierarchy.
- Use `events-aggregated` for comparison with Qt Creator.

# 10. Comparing Traces

Use `compare` to inspect hotspot differences between an initial-visit trace and
a revisit trace. Investigate causes afterward with `hotspots` and `ranges` on each trace.

Reference:

- `docs/TRACE_COMPARISON_GUIDE.md`

Approach:

- Match measurement conditions before comparing.
- Read each trace separately first.
- Use `compare before.qzt after.qzt` to view hotspot differences.
- Prioritize `hotspots` and `ranges` over `summary` when comparing.
- Prefer `groupKey` in `hotspots`, because `eventIndex` may not be stable across traces.
- Also use `display`, `file`, `line`, and `type` when needed.

# 11. Comparing with Qt Creator

Qt Creator and `qmlprofileranalyzer` do not use exactly the same display model,
but `events-aggregated` makes comparison easier.

Main correspondences:

- `PixmapCacheLoad`: Useful for comparison with Qt Creator's `Image Loaded`
- `SceneGraphFrame`: Useful for scene graph timeline comparison
- `MemoryAllocationSummary`: Useful for memory usage comparison
- `DebugMessageSummary`: Useful for debug message comparison

Suggested order:

1. `summary`
2. `hotspots`
3. `ranges`
4. `events-aggregated`
5. `events`, if needed

# 12. Demos

## 12.1 Minimal Demo

Fixed sample:

```text
samples/minimal.qtd
```

Example:

```text
$ ./build/qmlprofileranalyzer summary samples/minimal.qtd
$ ./build/qmlprofileranalyzer types samples/minimal.qtd
$ ./build/qmlprofileranalyzer notes samples/minimal.qtd
```

## 12.2 `qml` Runtime Demo

```text
$ scripts/generate_demo_trace.sh
$ ./build/qmlprofileranalyzer summary samples/generated/autotrace.qtd
```

Notes:

- Uses `qml` plus `qmlprofiler --attach`.
- Does not use `qmlscene`.
- May include `default.qml` events originating from the `qml` runtime.

## 12.3 Executable Demo

```text
$ scripts/generate_executable_demo_trace.sh
$ ./build/qmlprofileranalyzer summary samples/generated/localizedclockdemo.qtd
```

Notes:

- Launches `localizedclockdemo` and attaches to it.
- Intended for checks closer to real-world use.
- CMake builds `localizedclockdemo` only when `demos/localizedclock/main.cpp` exists.

## 12.4 Sample Analysis Report

The distribution includes a report analyzing sample traces:

- `samples/SAMPLES_TRACE_REPORT.md`
- `samples/SAMPLES_TRACE_REPORT.pdf`

It provides analysis examples for:

- `samples/minimal.qtd`
- `samples/generated/autotrace.qtd`
- `samples/generated/localizedclockdemo.qtd`

# 13. Basic Procedure for Evaluation Documents

When you have a `.qzt` file and its corresponding source code:

1. Use `summary` to inspect the overall volume.
2. Use `hotspots` to narrow down expensive locations.
3. Use `ranges` to check hierarchy and `selfTime`.
4. Use `types` to check code locations and types.
5. Use `events-aggregated` to inspect aggregates suitable for comparison.
6. Use `events` to inspect raw events if needed.
7. Map line numbers to the source code.

Related documents:

- `docs/SAMEGAME_QZT_PROCEDURE.md`
- `docs/QZT_EVALUATION_WORK_INSTRUCTIONS.md`
- `docs/QZT_EVALUATION_AGENT_INSTRUCTIONS.md`

# 14. Usage Notes

- Commands other than `compare` process one input file at a time.
- Analysis output is currently text only.
- As an exception, `convert` writes `.qtd` / `.qzt` trace files.
- Aggregate rows in `events-aggregated` are supplementary information.
- The tool does not exactly reproduce Qt Creator's UI or category names.
- `Compiling` may include imports and plugin loading, not just QML compilation.
- `gapWarning` flags time not readily explained by child QML ranges; it does not establish a cause.

# 15. Currently Unsupported Features

- JSON output
- CSV output
- Machine-readable structured output
- GUI
- Time-range filtering
- Type / file / line filtering
- Top-N output
- Detailed statistics by file / line
- Caller / callee analysis
- Flame-graph-style aggregation
- Large-file optimization

Multiple-trace comparison supports only hotspot differences through `compare`.
Range comparison and statistical significance testing are not supported.

For details, see:

- `docs/STATUS.md`
- `docs/ROADMAP.md`
