// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// ui_form.h を使い、.ui の objectName と C++ 側の参照名を確認する教材にしてください。
// go() 関数は widget.cpp に用意し、main() は書かないでください。

#pragma once

#include <QWidget>

#include "ui_form.h"

class Widget : public QWidget, private Ui::Form {
public:
    explicit Widget(QWidget *parent = nullptr);
};
