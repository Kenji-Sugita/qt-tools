// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// ui_form.h を使い、.ui の objectName と C++ 側の参照名を確認する教材にしてください。
// go() 関数を用意し、main() は書かないでください。

#include <QWidget>

#include "ui_form.h"

QWidget *go() {
    auto *widget = new QWidget;
    Ui::Form ui;
    ui.setupUi(widget);
    ui.messageLabel->setText("Fixed");
    return widget;
}
