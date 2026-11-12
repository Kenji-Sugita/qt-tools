#include "widget.h"
#include <QDebug>

Widget::Widget(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize Widget::sizeHint() const
{
    return QSize(150, 150);
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    qDebug().noquote() << QString("%1x%2").arg(width()).arg(height());
}
