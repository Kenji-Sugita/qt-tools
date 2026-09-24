# Executable Demo Outputs

## 1. トレース生成

### コマンド

```bash
scripts/generate_executable_demo_trace.sh
```

### 出力

```text
Generating trace: /Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/generated/localizedclockdemo.qtd
QML debugging is enabled. Only use this in a safe environment.
QML Debugger: Waiting for connection on port 37683...
qt.qpa.fonts: Populating font family aliases took 294 ms. Replace uses of missing font family "Sans Serif" with one that exists to avoid this cost.
This plugin does not support propagateSizeHints()
This plugin does not support propagateSizeHints()
Trace written to: /Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/generated/localizedclockdemo.qtd
```

## 2. summary

### コマンド

```bash
./build/qmlprofileranalyzer summary samples/generated/localizedclockdemo.qtd
```

### 出力

```text
file format: qtd/xml
version: 1.02
traceStart: 987099333
traceEnd: 16486268417
traceDuration: 15499169084
totalTime: 569050834
eventTypes: 37
events: 441
ranges: 106
pointEvents: 335
notes: 0
longestRange: idx=13 type=Main.qml:43 duration=472956334

per-type:
  [0] DebugMessage:1 count=3 totalDuration=0 maxDuration=0
  [1] MemoryAllocation:0 count=1 totalDuration=0 maxDuration=0
  [2] MemoryAllocation:2 count=247 totalDuration=0 maxDuration=0
  [3] MemoryAllocation:1 count=1 totalDuration=0 maxDuration=0
  [4] Main.qml:0 count=1 totalDuration=10541667 maxDuration=10541667
  [5] Main.qml:4 count=2 totalDuration=491501 maxDuration=427667
  [6] Main.qml:18 count=2 totalDuration=272875 maxDuration=139250
  [7] Main.qml:22 count=2 totalDuration=2221708 maxDuration=2151583
  [8] Main.qml:28 count=1 totalDuration=8500 maxDuration=8500
  [9] Main.qml:31 count=2 totalDuration=74792 maxDuration=66792
  [10] Main.qml:34 count=1 totalDuration=1083 maxDuration=1083
  [11] Main.qml:37 count=2 totalDuration=16000 maxDuration=9708
  [12] Main.qml:40 count=1 totalDuration=4125 maxDuration=4125
  [13] Main.qml:43 count=2 totalDuration=472958459 maxDuration=472956334
  [14] Main.qml:46 count=1 totalDuration=625 maxDuration=625
  [15] Main.qml:50 count=2 totalDuration=28834 maxDuration=19167
  [16] Main.qml:20 count=1 totalDuration=46917 maxDuration=46917
  [17] Main.qml:20 count=1 totalDuration=17458 maxDuration=17458
  [18] Main.qml:44 count=1 totalDuration=118750 maxDuration=118750
  [19] Main.qml:44 count=1 totalDuration=106916 maxDuration=106916
  [20] Main.qml:38 count=2 totalDuration=74815626 maxDuration=74808542
  [21] Main.qml:38 count=2 totalDuration=6875 maxDuration=6250
  [22] Main.qml:32 count=16 totalDuration=1747374 maxDuration=164625
  [23] Main.qml:32 count=16 totalDuration=268584 maxDuration=24292
  [24] Main.qml:23 count=2 totalDuration=199625 maxDuration=198458
  [25] Main.qml:23 count=2 totalDuration=2667 maxDuration=1792
  [26] Main.qml:9 count=3 totalDuration=87750 maxDuration=36875
  [27] Main.qml:9 count=3 totalDuration=48250 maxDuration=29750
  [28] Main.qml:10 count=3 totalDuration=23583 maxDuration=12291
  [29] Main.qml:10 count=3 totalDuration=14876 maxDuration=11584
  [30] Unknown count=1 totalDuration=1583 maxDuration=1583
  [31] Main.qml:56 count=15 totalDuration=82430122 maxDuration=76650708
  [32] Main.qml:56 count=15 totalDuration=82349914 maxDuration=76648500
  [33] SceneGraph:9 count=23 totalDuration=0 maxDuration=0
  [34] SceneGraph:0 count=23 totalDuration=0 maxDuration=0
  [35] SceneGraph:3 count=23 totalDuration=0 maxDuration=0
  [36] AnimationFrame count=14 totalDuration=0 maxDuration=0
```

## 3. types

### コマンド

```bash
./build/qmlprofileranalyzer types samples/generated/localizedclockdemo.qtd | sed -n '1,30p'
```

### 出力

```text
[0] type=DebugMessage display=DebugMessage:1
[1] type=MemoryAllocation display=MemoryAllocation:0 memoryEventType=0
[2] type=MemoryAllocation display=MemoryAllocation:2 memoryEventType=2
[3] type=MemoryAllocation display=MemoryAllocation:1 memoryEventType=1
[4] type=Compiling display=Main.qml:0 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:0:0 details=Main.qml
[5] type=Creating display=Main.qml:4 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:4:1 details=QtQuick/Window
[6] type=Creating display=Main.qml:18 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:18:5 details=QtQuick.Layouts/ColumnLayout
[7] type=Creating display=Main.qml:22 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:22:9 details=QtQuick/Text
[8] type=Creating display=Main.qml:28 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:28:13 details=QtQuick.Layouts/Layout
[9] type=Creating display=Main.qml:31 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:31:9 details=QtQuick/Text
[10] type=Creating display=Main.qml:34 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:34:13 details=QtQuick.Layouts/Layout
[11] type=Creating display=Main.qml:37 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:37:9 details=QtQuick/Text
[12] type=Creating display=Main.qml:40 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:40:13 details=QtQuick.Layouts/Layout
[13] type=Creating display=Main.qml:43 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:43:9 details=QtQuick/Text
[14] type=Creating display=Main.qml:46 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:46:13 details=QtQuick.Layouts/Layout
[15] type=Creating display=Main.qml:50 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:50:5 details=QtQml/Timer
[16] type=Binding display=Main.qml:20 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:20:9 bindingType=-1
[17] type=Javascript display=Main.qml:20 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:20:9 details=expression for fill
[18] type=Binding display=Main.qml:44 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:44:13 bindingType=-1
[19] type=Javascript display=Main.qml:44 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:44:13 details=expression for text
[20] type=Binding display=Main.qml:38 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:38:13 bindingType=-1
[21] type=Javascript display=Main.qml:38 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:38:13 details=expression for text
[22] type=Binding display=Main.qml:32 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:32:13 bindingType=-1
[23] type=Javascript display=Main.qml:32 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:32:13 details=expression for text
[24] type=Binding display=Main.qml:23 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:23:13 bindingType=-1
[25] type=Javascript display=Main.qml:23 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:23:13 details=expression for text
[26] type=Binding display=Main.qml:9 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:9:5 bindingType=-1
[27] type=Javascript display=Main.qml:9 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:9:5 details=expression for minimumWidth
[28] type=Binding display=Main.qml:10 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:10:5 bindingType=-1
[29] type=Javascript display=Main.qml:10 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/demos/localizedclock/Main.qml:10:5 details=expression for minimumHeight
```
