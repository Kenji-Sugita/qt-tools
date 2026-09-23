#!/bin/sh

QT_ROOT=/usr/local/qt/Qt/6.11.0/macos
QT_LIB=${QT_ROOT}/lib

# OK
if true ; then
    qtcling -c startup.cpp \
        -I ${QT_LIB}/QtCore5Compat.framework/Headers \
        ${QT_LIB}/QtCore5Compat.framework/Versions/A/QtCore5Compat
fi

# OK
if false ; then
qtcling -c startup.cpp \
        -I ${QT_LIB}/QtCore.framework/Headers \
        -I ${QT_LIB}/QtCore5Compat.framework/Headers \
        -L ${QT_LIB}/QtCore5Compat.framework/Versions/A \
        -l QtCore5Compat
fi

# NOT OK、QtTest では OK
if false ; then
    qtcling -c startup.cpp \
        -I ${QT_LIB}/QtCore.framework/Headers \
        -I ${QT_LIB}/QtCore5Compat.framework/Headers \
        -F ${QT_LIB} \
        -framework QtCore5Compat
fi
