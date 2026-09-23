// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// ui_form.h を使って form.ui だけで QWidget を作成してください。
// go() 関数を用意し、main() は書かないでください。

#include <QWidget>

#include "ui_form.h"

QWidget *go() {
    auto *widget = new QWidget;
    Ui::Form ui;
    ui.setupUi(widget);
    return widget;
}
