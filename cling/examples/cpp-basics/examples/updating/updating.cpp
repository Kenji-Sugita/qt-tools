#include "updating.h"
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QApplication>
#include <QMetaObject>
#include <QDebug>
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#include <stdlib.h>
#endif

Updating::Updating(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(sizeHint());
}

QSize Updating::sizeHint() const
{
    return QSize(3*sideLength, sideLength);
}

void Updating::paintEvent(QPaintEvent* event)
{
    qDebug() << Q_FUNC_INFO << event->rect() << event->region();

    if (event->rect() == rect()) {
        // Force update later
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        QMetaObject::invokeMethod(this, &Updating::updating, Qt::QueuedConnection);
#else
        QMetaObject::invokeMethod(this, "updating", Qt::QueuedConnection);
#endif
        return;
    }

    QPainter painter(this);
    for (const QRect& smallRect : event->region()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        painter.fillRect(smallRect, QColor(rand() % 256, rand() % 256, rand() % 256));
#else
        painter.fillRect(smallRect, QColor(qrand() % 256, qrand() % 256, qrand() % 256));
#endif
    }
}

void Updating::updating()
{
    const QPushButton* const button = qobject_cast<QPushButton*>(sender());  // Sender is nullptr or a QObject

    auto updater = QOverload<const QRect&>::of(&Updating::update);
    if (button && button->text() == "Repaint") {
        updater = QOverload<const QRect&>::of(&Updating::repaint);
    }

    for (int row = 0; row < rect().height() - smallRectSideLength; row += 10) {
        for (int column = 0; column < rect().width() - smallRectSideLength; column += 10) {
            (this->*updater)(QRect(column, row, smallRectSideLength, smallRectSideLength));
        }
    }
}
