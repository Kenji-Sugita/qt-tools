#include "FloatingActionBubbleLabel.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>

FloatingActionBubbleLabel::FloatingActionBubbleLabel(QWidget *parent)
    : QLabel(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAlignment(Qt::AlignCenter);
}

QSize FloatingActionBubbleLabel::sizeHint() const
{
    const QFontMetrics metrics(font());
    return QSize(metrics.horizontalAdvance(text()) + 42, qMax(34, metrics.height() + 14));
}

void FloatingActionBubbleLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    constexpr int pointerWidth = 12;
    constexpr int shadowOffset = 3;
    const QRectF bodyRect(1.0, 1.0, width() - pointerWidth - shadowOffset - 2.0, height() - shadowOffset - 2.0);
    const QPointF tip(width() - shadowOffset - 1.0, bodyRect.center().y());

    QPainterPath path;
    path.addRoundedRect(bodyRect, 5.0, 5.0);
    path.moveTo(bodyRect.right() - 1.0, bodyRect.center().y() - 8.0);
    path.lineTo(tip);
    path.lineTo(bodyRect.right() - 1.0, bodyRect.center().y() + 8.0);
    path.closeSubpath();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath shadowPath = path.translated(shadowOffset, shadowOffset);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(15, 23, 42, 60));
    painter.drawPath(shadowPath);

    painter.setPen(QColor(15, 23, 42, 220));
    painter.setBrush(QColor(15, 23, 42, 235));
    painter.drawPath(path);

    painter.setPen(Qt::white);
    QFont textFont = font();
    textFont.setWeight(QFont::Bold);
    painter.setFont(textFont);
    painter.drawText(bodyRect.adjusted(11.0, 0.0, -8.0, 0.0), Qt::AlignCenter, text());
}
