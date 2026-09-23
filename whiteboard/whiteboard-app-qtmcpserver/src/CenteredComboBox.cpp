#include "CenteredComboBox.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOptionComboBox>

CenteredComboBox::CenteredComboBox(QWidget *parent)
    : QComboBox(parent)
{
}

void CenteredComboBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOptionComboBox option;
    initStyleOption(&option);

    QPainter painter(this);
    style()->drawComplexControl(QStyle::CC_ComboBox, &option, &painter, this);

    QRect textRect = style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this);
    textRect.adjust(8, 0, 0, 0);
    style()->drawItemText(&painter, textRect, Qt::AlignCenter, palette(), isEnabled(), currentText(), QPalette::Text);
}
