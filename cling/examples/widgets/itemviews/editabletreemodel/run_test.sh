#!/bin/sh

QT_ROOT=/usr/local/qt/Qt/6.11.0/macos
QT_LIB=${QT_ROOT}/lib

# OK
if true ; then
    qtcling -c startup_test.cpp -I ${QT_LIB}/QtTest.framework/Headers \
                                ${QT_LIB}/QtTest.framework/Versions/A/QtTest
fi

# OK
if false ; then
    qtcling -c startup_test.cpp -I ${QT_LIB}/QtTest.framework/Headers \
                                -L ${QT_LIB}/QtTest.framework/Versions/A \
                                -l QtTest
fi
