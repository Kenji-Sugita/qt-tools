TEMPLATE = app
TARGET = container
INCLUDEPATH += .

SOURCES += main.cpp object.cpp
HEADERS += object.h
macos:CONFIG-=app_bundle
