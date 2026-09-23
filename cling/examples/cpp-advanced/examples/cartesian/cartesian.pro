QT += widgets
TEMPLATE = app
TARGET = cartesian
INCLUDEPATH += .

macos:CONFIG -= app_bundle
win32:CONFIG += console

SOURCES += main.cpp
