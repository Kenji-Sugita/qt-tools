# Demo Outputs

## 1. 最小デモ

### コマンド

```bash
./build/qmlprofileranalyzer summary samples/minimal.qtd
```

### 出力

```text
file format: qtd/xml
version: 1.02
traceStart: 1000
traceEnd: 1800
traceDuration: 800
totalTime: 260
eventTypes: 3
events: 3
ranges: 1
pointEvents: 2
notes: 1
longestRange: idx=0 type=root width binding duration=40

per-type:
  [0] root width binding count=1 totalDuration=40 maxDuration=40
  [1] Animation Frame count=1 totalDuration=0 maxDuration=0
  [2] demo warning count=1 totalDuration=0 maxDuration=0
```

### コマンド

```bash
./build/qmlprofileranalyzer types samples/minimal.qtd
```

### 出力

```text
[0] type=Binding display=root width binding file=samples/qml/AutoTrace.qml:17:13 bindingType=0 details=root.width
[1] type=Event display=Animation Frame animationFrame=3
[2] type=DebugMessage display=demo warning level=1
```

### コマンド

```bash
./build/qmlprofileranalyzer notes samples/minimal.qtd
```

### 出力

```text
1100  duration=40  idx=0  row=0  text=Investigate this binding first.
```

## 2. 実行デモ

### トレース生成コマンド

```bash
scripts/generate_demo_trace.sh
```

### 生成結果

```text
Generating trace: /Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/generated/autotrace.qtd
QML Debugger: Waiting for connection on port 37680...
qt.qpa.fonts: Populating font family aliases took 318 ms. Replace uses of missing font family "Sans Serif" with one that exists to avoid this cost.
Trace written to: /Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/generated/autotrace.qtd
```

### コマンド

```bash
./build/qmlprofileranalyzer summary samples/generated/autotrace.qtd
```

### 出力

```text
file format: qtd/xml
version: 1.02
traceStart: 933498542
traceEnd: 2661200084
traceDuration: 1727701542
totalTime: 518638710
eventTypes: 26
events: 423
ranges: 24
pointEvents: 399
notes: 0
longestRange: idx=12 type=AutoTrace.qml:35 duration=508066625

per-type:
  [0] MemoryAllocation:0 count=2 totalDuration=0 maxDuration=0
  [1] MemoryAllocation:2 count=91 totalDuration=0 maxDuration=0
  [2] MemoryAllocation:1 count=2 totalDuration=0 maxDuration=0
  [3] default.qml:0 count=1 totalDuration=495333 maxDuration=495333
  [4] default.qml:5 count=1 totalDuration=37250 maxDuration=37250
  [5] default.qml:6 count=1 totalDuration=17708 maxDuration=17708
  [6] DebugMessage:1 count=1 totalDuration=0 maxDuration=0
  [7] AutoTrace.qml:0 count=1 totalDuration=8228792 maxDuration=8228792
  [8] AutoTrace.qml:4 count=2 totalDuration=571459 maxDuration=500667
  [9] AutoTrace.qml:11 count=2 totalDuration=35250 maxDuration=33916
  [10] AutoTrace.qml:17 count=2 totalDuration=35167 maxDuration=33542
  [11] AutoTrace.qml:26 count=2 totalDuration=73834 maxDuration=49125
  [12] AutoTrace.qml:35 count=2 totalDuration=508183917 maxDuration=508066625
  [13] AutoTrace.qml:43 count=2 totalDuration=33291 maxDuration=18875
  [14] AutoTrace.qml:36 count=1 totalDuration=38584 maxDuration=38584
  [15] AutoTrace.qml:36 count=1 totalDuration=27458 maxDuration=27458
  [16] AutoTrace.qml:37 count=1 totalDuration=7125 maxDuration=7125
  [17] AutoTrace.qml:37 count=1 totalDuration=6000 maxDuration=6000
  [18] AutoTrace.qml:13 count=1 totalDuration=1792 maxDuration=1792
  [19] AutoTrace.qml:13 count=1 totalDuration=708 maxDuration=708
  [20] SceneGraph:9 count=76 totalDuration=0 maxDuration=0
  [21] SceneGraph:0 count=76 totalDuration=0 maxDuration=0
  [22] SceneGraph:3 count=76 totalDuration=0 maxDuration=0
  [23] AnimationFrame count=75 totalDuration=0 maxDuration=0
  [24] AutoTrace.qml:47 count=1 totalDuration=1125250 maxDuration=1125250
  [25] AutoTrace.qml:47 count=1 totalDuration=1117167 maxDuration=1117167
```

### コマンド

```bash
./build/qmlprofileranalyzer types samples/generated/autotrace.qtd | sed -n '1,20p'
```

### 出力

```text
[0] type=MemoryAllocation display=MemoryAllocation:0 memoryEventType=0
[1] type=MemoryAllocation display=MemoryAllocation:2 memoryEventType=2
[2] type=MemoryAllocation display=MemoryAllocation:1 memoryEventType=1
[3] type=Compiling display=default.qml:0 file=qrc:/qt-project.org/imports/QmlRuntime/Config/default.qml:0:0 details=qrc:/qt-project.org/imports/QmlRuntime/Config/default.qml
[4] type=Creating display=default.qml:5 file=qrc:/qt-project.org/imports/QmlRuntime/Config/default.qml:5:1 details=QmlRuntime.Config/Configuration
[5] type=Creating display=default.qml:6 file=qrc:/qt-project.org/imports/QmlRuntime/Config/default.qml:6:5 details=QmlRuntime.Config/PartialScene
[6] type=DebugMessage display=DebugMessage:1
[7] type=Compiling display=AutoTrace.qml:0 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:0:0 details=AutoTrace.qml
[8] type=Creating display=AutoTrace.qml:4 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:4:1 details=QtQuick/Window
[9] type=Creating display=AutoTrace.qml:11 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:11:5 details=QtQuick/Rectangle
[10] type=Creating display=AutoTrace.qml:17 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:17:5 details=QtQuick/Rectangle
[11] type=Creating display=AutoTrace.qml:26 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:26:9 details=QtQuick/NumberAnimation
[12] type=Creating display=AutoTrace.qml:35 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:35:5 details=QtQuick/Text
[13] type=Creating display=AutoTrace.qml:43 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:43:5 details=QtQml/Timer
[14] type=Binding display=AutoTrace.qml:36 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:36:9 bindingType=-1
[15] type=Javascript display=AutoTrace.qml:36 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:36:9 details=expression for horizontalCenter
[16] type=Binding display=AutoTrace.qml:37 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:37:9 bindingType=-1
[17] type=Javascript display=AutoTrace.qml:37 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:37:9 details=expression for bottom
[18] type=Binding display=AutoTrace.qml:13 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:13:9 bindingType=-1
[19] type=Javascript display=AutoTrace.qml:13 file=file:///Users/sugita/src/qt/tools/qmlprofileranalyzer/samples/qml/AutoTrace.qml:13:9 details=expression for fill
```
