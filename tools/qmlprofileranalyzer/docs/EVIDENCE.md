# Evidence Pointers

## Qt `qmlprofiler`

- XML 保存本体
  - `/usr/local/qt/qt6.10.1-src/qtdeclarative/tools/qmlprofiler/qmlprofilerdata.cpp:502`
- file version と type 名文字列
  - `/usr/local/qt/qt6.10.1-src/qtdeclarative/tools/qmlprofiler/qmlprofilerdata.cpp:15`
- Qt Creator で可視化できる旨
  - `/usr/local/qt/qt6.10.1-src/qtdeclarative/tools/qmlprofiler/qmlprofilerapplication.cpp:114`
- profiler enum 定義
  - `/usr/local/qt/qt6.10.1-src/qtdeclarative/src/qmldebug/qqmlprofilerclientdefinitions_p.h:23`

## Qt Creator

- 保存形式の分岐 `.qtd` / `.qzt`
  - `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertracefile.cpp:109`
- `.qtd` XML reader
  - `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertracefile.cpp:127`
- `.qzt` binary reader
  - `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertracefile.cpp:178`
- XML `eventData` reader
  - `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertracefile.cpp:285`
- XML `profilerDataModel` reader
  - `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertracefile.cpp:478`
- XML writer `.qtd`
  - `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertracefile.cpp:606`
- binary writer `.qzt`
  - `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertracefile.cpp:777`
- file extension 定数
  - `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilerconstants.h:16`
- GUI 既定保存拡張子が `.qzt`
  - `/usr/local/qt/qt-creator-src/src/plugins/qmlprofiler/qmlprofilertool.cpp:609`
- trace manager の load/save 起点
  - `/usr/local/qt/qt-creator-src/src/libs/tracing/timelinetracemanager.cpp:214`
