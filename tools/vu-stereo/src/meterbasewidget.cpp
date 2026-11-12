// MeterBaseWidget の実装です。暗い金属調の台座、上部ハイライト、下部の影を
// QPainter で描画します。透明ウィンドウの外側には何も塗らず、台座部分だけが
// 見えるようにするための外観専用クラスです。

#include "meterbasewidget.h"

#include <QLinearGradient>
#include <QPainter>

MeterBaseWidget::MeterBaseWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAttribute(Qt::WA_TranslucentBackground);
}

void MeterBaseWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF bounds = rect().adjusted(4.0, 4.0, -4.0, -4.0);
    const QRectF top = bounds.adjusted(8.0, 8.0, -8.0, -bounds.height() * 0.42);
    const QRectF foot = bounds.adjusted(0.0, bounds.height() * 0.50, 0.0, 0.0);

    QLinearGradient bodyGradient(bounds.topLeft(), bounds.bottomLeft());
    bodyGradient.setColorAt(0.0, QColor(54, 55, 51));
    bodyGradient.setColorAt(0.22, QColor(22, 23, 21));
    bodyGradient.setColorAt(0.68, QColor(34, 35, 32));
    bodyGradient.setColorAt(1.0, QColor(8, 9, 8));

    painter.setPen(QPen(QColor(4, 4, 4), 2.0));
    painter.setBrush(bodyGradient);
    painter.drawRoundedRect(bounds, 18.0, 18.0);

    QLinearGradient footGradient(foot.topLeft(), foot.bottomLeft());
    footGradient.setColorAt(0.0, QColor(41, 42, 38));
    footGradient.setColorAt(1.0, QColor(5, 5, 5));
    painter.setPen(QPen(QColor(0, 0, 0, 180), 1.5));
    painter.setBrush(footGradient);
    painter.drawRoundedRect(foot, 14.0, 14.0);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 38));
    painter.drawRoundedRect(top, 10.0, 10.0);

    painter.setBrush(QColor(0, 0, 0, 90));
    painter.drawRoundedRect(bounds.adjusted(18.0, bounds.height() - 18.0, -18.0, -5.0), 6.0, 6.0);
}
