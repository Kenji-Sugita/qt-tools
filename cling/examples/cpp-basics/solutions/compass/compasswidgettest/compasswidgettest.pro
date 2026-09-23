QT += widgets

TEMPLATE    = app
TARGET      = compasswidgettest
DEPENDPATH  += . ../compasswidget
INCLUDEPATH += . ../compasswidget

DEFINES += COMPASSWIDGETAPP

SOURCES += main.cpp
SOURCES += ../compasswidget/compasswidget.cpp ../compasswidget/compasswidget2.cpp
HEADERS += ../compasswidget/compasswidget.h ../compasswidget/compasswidget2.h
HEADERS += ../compasswidget/direction.h 
