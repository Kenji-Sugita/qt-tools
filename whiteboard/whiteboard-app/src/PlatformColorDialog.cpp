#include "PlatformColorDialog.h"

#include <QColorDialog>
#include <QCoreApplication>
#include <QtGlobal>

namespace {
QColor fillDialogInitialColor(QColor color)
{
#ifdef Q_OS_WIN
    if (color.isValid() && color.alpha() == 0)
        color.setAlpha(255);
#endif
    return color;
}
}

QColor PlatformColorDialog::getFillColor(const QColor &initialColor, QWidget *parent)
{
    return QColorDialog::getColor(
        fillDialogInitialColor(initialColor),
        parent,
        QCoreApplication::translate("MainWindow", "Fill"),
        QColorDialog::ShowAlphaChannel);
}
