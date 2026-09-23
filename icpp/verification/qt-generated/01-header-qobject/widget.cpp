#include "widget.h"

Widget *go() {
    auto *widget = new Widget;
    widget->resize(widget->sizeHint());
    return widget;
}

#include "moc_widget.cpp"

