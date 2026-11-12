QT += widgets
TEMPLATE =app
SOURCES	+= findDialog.cpp main.cpp
HEADERS	+= findDialog.h
FORMS	= findDialog.ui

!unix:error(Cannot use the program on this platform)
