QT       += widgets uiplugin
TEMPLATE  = lib
CONFIG   += plugin

TARGET    = colorwidgetplugin

HEADERS  += \
    colorwidget.h \
    colorwidgetplugin.h

SOURCES  += \
    colorwidget.cpp \
    colorwidgetplugin.cpp

FORMS   += \
    colorwidget.ui

OTHER_FILES += \
    colorwidget.json

macx {
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}
