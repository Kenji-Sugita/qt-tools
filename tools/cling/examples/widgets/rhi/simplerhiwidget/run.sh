#!/bin/sh

QT_ROOT=/usr/local/qt/Qt/6.11.0/macos
QT_LIB=${QT_ROOT}/lib

# rhi/qrhi.h をインクルードするために必要
qtcling -c startup.cpp \
    -I ${QT_LIB}/QtGui.framework/Versions/A/Headers/6.11.0/QtGui
