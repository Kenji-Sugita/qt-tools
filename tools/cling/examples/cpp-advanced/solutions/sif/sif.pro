TEMPLATE = lib

CONFIG += plugin

HEADERS += SimpleImageIOHandler.h SimpleImagePlugin.h
SOURCES += SimpleImageIOHandler.cpp SimpleImagePlugin.cpp

OTHER_FILES += sif.json

target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target
