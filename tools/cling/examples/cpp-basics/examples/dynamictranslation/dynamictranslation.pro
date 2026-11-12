QT += widgets

TEMPLATE = app
TARGET = dynamictranslation
INCLUDEPATH += .

SOURCES += \
    harness.cpp \
    main.cpp

TRANSLATIONS += dynamictranslation_en.ts dynamictranslation_ja.ts
RESOURCES += dynamictranslation.qrc

HEADERS += \
    harness.h

OTHER_FILES += \
    dynamictranslation_en.ts \
    dynamictranslation_ja.ts
