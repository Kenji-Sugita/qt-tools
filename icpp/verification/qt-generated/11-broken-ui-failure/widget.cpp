#include <QWidget>

#include "ui_form.h"

QWidget *go() {
    auto *widget = new QWidget;
    Ui::Form ui;
    ui.setupUi(widget);
    return widget;
}
