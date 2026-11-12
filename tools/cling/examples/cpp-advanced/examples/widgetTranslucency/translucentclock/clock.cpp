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

#include "clock.h"
#include <QTime>
#include <QTimer>
#include <QAction>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>

Clock::Clock(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, static_cast<void (Clock::*)()>(&Clock::update));
    timer->start(100);

    auto quitAction = new QAction(tr("E&xit"), this);
    quitAction->setShortcut(tr("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    addAction(quitAction);

    setAttribute(Qt::WA_TranslucentBackground);
    setContextMenuPolicy(Qt::ActionsContextMenu);
    setToolTip(tr("Drag the clock with the left mouse button.\n"
                  "Use the right mouse button to open a context menu."));
    setWindowTitle(tr("Analog Clock"));
}

void Clock::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        dragPosition = event->position().toPoint() - frameGeometry().topLeft();
#else
        dragPosition = event->globalPos() - frameGeometry().topLeft();
#endif
        event->accept();
    }
}

void Clock::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        move(event->position().toPoint() - dragPosition);
#else
        move(event->globalPos() - dragPosition);
#endif
        event->accept();
    }
}

void Clock::paintEvent(QPaintEvent *)
{
    const QPoint hourHand[3] {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -40)
    };
    const QPoint minuteHand[3] {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -70)
    };

    const QColor hourColor(191, 0, 0);
    const QColor minuteColor(0, 0, 191, 191);

    const int side = qMin(width(), height());
    const QTime time = QTime::currentTime();

    // Draw clock face
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);

    QRadialGradient gradient(0.0, 0.0, side * 0.5, 0.0, 0.0);
    gradient.setColorAt(0.0, QColor(255, 255, 255, 255));
    gradient.setColorAt(0.1, QColor(255, 255, 255, 31));
    gradient.setColorAt(0.7, QColor(255, 255, 255, 31));
    gradient.setColorAt(0.8, QColor(0, 31, 0, 31));
    gradient.setColorAt(0.9, QColor(255, 255, 255, 255));
    gradient.setColorAt(1.0, QColor(255, 255, 255, 255));
    painter.setPen(QColor(0, 0, 0, 32));
    painter.setBrush(gradient);
    painter.drawEllipse(static_cast<int>(-side/2.0) + 1, static_cast<int>(-side/2.0) + 1, side - 2, side - 2);

    painter.scale(side / 200.0, side / 200.0);

    // Draw hour hand
    painter.setPen(Qt::NoPen);
    painter.setBrush(hourColor);

    painter.save();
    painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
    painter.drawConvexPolygon(hourHand, 3);
    painter.restore();

    painter.setPen(hourColor);

    // Draw clock scale
    for (int i = 0; i < 12; ++i) {
        painter.drawLine(88, 0, 96, 0);
        painter.rotate(30.0);
    }

    // Draw minite hand
    painter.setPen(Qt::NoPen);
    painter.setBrush(minuteColor);

    painter.save();
    painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
    painter.drawConvexPolygon(minuteHand, 3);
    painter.restore();

    painter.setPen(minuteColor);

    // Draw clock scale
    for (int j = 0; j < 60; ++j) {
        if ((j % 5) != 0) {
            painter.drawLine(92, 0, 96, 0);
        }
        painter.rotate(6.0);
    }
}

QSize Clock::sizeHint() const
{
    return QSize(200, 200);
}
