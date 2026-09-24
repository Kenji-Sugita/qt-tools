# Sources

## ローカル参照元

Qt `qmlprofiler`:

- `/usr/local/qt/qt6.10.1-src/qtdeclarative/tools/qmlprofiler/qmlprofilerapplication.cpp`
- `/usr/local/qt/qt6.10.1-src/qtdeclarative/tools/qmlprofiler/qmlprofilerdata.cpp`
- `/usr/local/qt/qt6.10.1-src/qtdeclarative/tools/qmlprofiler/qmlprofilerdata.h`
- `/usr/local/qt/qt6.10.1-src/qtdeclarative/src/qmldebug/qqmlprofilerclientdefinitions_p.h`

Qt Creator:

- `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertracefile.cpp`
- `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertracefile.h`
- `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilerconstants.h`
- `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertool.cpp`
- `/usr/local/qt/qt-creator-src/src/libs/tracing/timelinetracemanager.cpp`
- `/usr/local/qt/qt-creator-src/src/libs/tracing/timelinetracemanager.h`
- `/usr/local/qt/qt-creator-src/src/libs/tracing/timelinetracefile.h`

## 保存した参照コピー

Qt `qmlprofiler`:

- `references/qtdeclarative/qmlprofilerapplication.cpp`
- `references/qtdeclarative/qmlprofilerdata.cpp`
- `references/qtdeclarative/qmlprofilerdata.h`
- `references/qtdeclarative/qqmlprofilerclientdefinitions_p.h`

Qt Creator:

- `references/qtcreator/qmlprofiler/qmlprofilertracefile.cpp`
- `references/qtcreator/qmlprofiler/qmlprofilertracefile.h`
- `references/qtcreator/qmlprofiler/qmlprofilerconstants.h`
- `references/qtcreator/qmlprofiler/qmlprofilertool.cpp`
- `references/qtcreator/tracing/timelinetracemanager.cpp`
- `references/qtcreator/tracing/timelinetracemanager.h`
- `references/qtcreator/tracing/timelinetracefile.h`

## Qt Creator clone 情報

- clone path: `/usr/local/qt/qt-creator-src`
- shallow clone date: 2026-04-22
- HEAD: `2eadb86ded828d18ade0a60a1a984131ee55fccf`

## Web 出典

- Qt Creator documentation
  - <https://doc.qt.io/qtcreator/creator-how-to-profile-qml.html>
- Qt Qml `qmlprofiler`
  - <https://doc.qt.io/qt-6/qtqml-tooling-qmlprofiler.html>
- Qt Quick profiling overview
  - <https://doc.qt.io/qt-6/qtquick-profiling.html>

## 調査で確定した点

- Qt Creator GUI の既定保存拡張子は `.qzt`
- `.qtd` は XML
- `.qzt` は `QDataStream` + `qCompress()`
- Qt CLI `qmlprofiler` の XML は Qt Creator の `.qtd` reader で読める設計
