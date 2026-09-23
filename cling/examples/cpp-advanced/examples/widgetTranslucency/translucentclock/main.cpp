/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

#include <QApplication>

#include "clock.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Clock clock;
    clock.show();

    return app.exec();
}
