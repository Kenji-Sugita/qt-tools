# QML Profiler Format Notes

## 1. 形式の切り分け

- Qt `qmlprofiler` CLI は XML を出力する
- Qt Creator は拡張子で分岐する
- `.qtd` は XML
- `.qzt` は `QDataStream` バイナリ

Qt Creator の実装:

- `QmlProfilerTraceFile::load()`
  - `.qtd` なら `loadQtd()`
  - それ以外は `loadQzt()`
- `QmlProfilerTraceFile::save()`
  - `.qtd` なら `saveQtd()`
  - それ以外は `saveQzt()`

## 2. `.qtd` / CLI XML

### 2.1 ルート

```xml
<trace version="1.02" traceStart="..." traceEnd="...">
  <eventData totalTime="...">
    ...
  </eventData>
  <profilerDataModel>
    ...
  </profilerDataModel>
  <noteData>
    ...
  </noteData>
</trace>
```

補足:

- Qt CLI 側は通常 `eventData` と `profilerDataModel` を出力する
- Qt Creator XML は `noteData` も扱う

### 2.2 `eventData/event`

主な要素:

- `displayname`
- `type`
- `filename`
- `line`
- `column`
- `details`
- `bindingType`
- `animationFrame`
- `keyEvent`
- `mouseEvent`
- `cacheEventType`
- `sgEventType`
- `memoryEventType`
- `level`

`type` は range 名または message 名:

- Range: `Painting`, `Compiling`, `Creating`, `Binding`, `HandlingSignal`, `Javascript`
- Message: `Event`, `RangeStart`, `RangeData`, `RangeLocation`, `RangeEnd`, `Complete`, `PixmapCache`, `SceneGraph`, `MemoryAllocation`, `DebugMessage`, `Quick3D`

### 2.3 `profilerDataModel/range`

共通属性:

- `startTime`
- `eventIndex`
- `duration` (range のとき)

イベント種別ごとの属性:

- animation frame: `framerate`, `animationcount`, `thread`
- input event: `type`, `data1`, `data2`
- pixmap size known: `width`, `height`
- pixmap ref/cache count changed: `refCount`
- scene graph: `timing1` ... `timing5`
- memory allocation: `amount`
- debug message: `text`

### 2.4 `noteData/note`

属性:

- `startTime`
- `duration`
- `eventIndex`
- `collapsedRow`

本文:

- ノート文字列

## 3. `.qzt` バイナリ

### 3.1 ヘッダー

`QDataStream` で次を順に読む:

1. magic: `QMLPROFILER`
2. `qint32 dataStreamVersion`
3. `qint64 traceStart`
4. `qint64 traceEnd`

その後、`dataStreamVersion` に合わせて stream version を切り替える。

### 3.2 本体ブロック

各ブロックは `QByteArray` として読み出され、`qUncompress()` で展開される。

順序:

1. event types ブロック
2. notes ブロック
3. event ブロック群

event types ブロック:

- `quint32 numEventTypes`
- その後に `QmlEventType` が `numEventTypes` 個

notes ブロック:

- `QList<QmlNote>`

event ブロック群:

- 複数の圧縮ブロック
- 各ブロック内に `QmlEvent` を連続格納
- Qt Creator 保存側は約 32 MiB ごとに区切って flush

### 3.3 実装上の意味

`.qzt` を読むには最低限次が必要:

- `QDataStream` のレイアウト理解
- `QmlEventType` の直列化形式
- `QmlNote` の直列化形式
- `QmlEvent` の直列化形式

そのため、解析ツールの最初の対応対象としては `.qtd` の方が実装負荷が低い。

## 4. 主要 enum

主要 enum は `qqmlprofilerclientdefinitions_p.h` にある。

- `Message`
- `EventType`
- `RangeType`
- `PixmapEventType`
- `SceneGraphFrameType`
- `MemoryType`
- `ProfileFeature`
- `InputEventType`

## 5. 実装方針

最小実装:

1. `.qtd` XML parser
2. event type table の構築
3. `range` の復元
4. note 読み込み

次段階:

1. `.qzt` reader
2. Qt Creator と同等の event ordering 復元
3. 必要なら `.qzt` writer
