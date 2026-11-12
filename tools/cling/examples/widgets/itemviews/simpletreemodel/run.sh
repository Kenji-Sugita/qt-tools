#!/bin/sh

QT_ROOT=/usr/local/qt/Qt/6.11.0/macos
QT_LIB=${QT_ROOT}/lib

# OK
if true ; then
    qtcling -c startup.cpp \
        -I ${QT_LIB}/QtTest.framework/Headers \
        -F ${QT_LIB} \
        -framework QtTest
fi
