#!/bin/sh

QT_ROOT=/usr/local/qt/Qt/6.11.0/macos
QT_LIB=${QT_ROOT}/lib

# OK
if true ; then
    qtcling -c startup.cpp -I ${QT_LIB}/QtPrintSupport.framework/Headers \
                           ${QT_LIB}/QtPrintSupport.framework/Versions/A/QtPrintSupport
fi

# OK
if false ; then
    qtcling -c startup.cpp -I ${QT_LIB}/QtPrintSupport.framework/Headers \
                           -L ${QT_LIB}/QtPrintSupport.framework/Versions/A \
                           -l QtPrintSupport
fi
