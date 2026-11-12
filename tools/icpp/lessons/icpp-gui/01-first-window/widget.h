// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QLabel を表示する header-only Widget クラスを定義してください。
// main() は書かず、REPL で widget.cpp の go() から使える形にしてください。

#pragma once

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class Widget : public QWidget {
public:
    explicit Widget(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("01 Header-only Widget");

        auto *label = new QLabel("Hello icpp", this);
        label->setAlignment(Qt::AlignCenter);

        auto *layout = new QVBoxLayout(this);
        layout->addWidget(label);

        resize(260, 120);
    }
};
