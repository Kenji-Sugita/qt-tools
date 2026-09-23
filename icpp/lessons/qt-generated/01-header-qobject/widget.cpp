// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// widget.h の Q_OBJECT 付き Widget を作成して返す go() 関数を用意してください。
// moc_widget.cpp を include し、main() は書かないでください。

#include "widget.h"

Widget *go() {
    auto *widget = new Widget;
    widget->resize(widget->sizeHint());
    return widget;
}

#include "moc_widget.cpp"

