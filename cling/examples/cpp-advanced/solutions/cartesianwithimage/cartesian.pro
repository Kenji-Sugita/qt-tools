QT += widgets
TEMPLATE = app
TARGET = cartesianwithimage
INCLUDEPATH += .

macos:CONFIG -= app_bundle
win32:CONFIG += console

SOURCES += main.cpp

RESOURCES += \
    cartesian.qrc
