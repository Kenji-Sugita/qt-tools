// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// Widget クラスで ui_form.h を setupUi し、.ui の objectName と C++ 側の参照名を確認する教材にしてください。
// go() 関数を用意し、main() は書かないでください。

#include "widget.h"

Widget::Widget(QWidget *parent) : QWidget(parent) {
    setupUi(this);
    messageLabel->setText("Fixed");
}

Widget *go() {
    return new Widget;
}
