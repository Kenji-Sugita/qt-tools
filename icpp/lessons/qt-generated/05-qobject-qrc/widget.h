// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// Q_OBJECT を持つ Widget クラスを宣言してください。
// main() は書かず、qrc と moc を組み合わせる教材として使える形にしてください。

#pragma once

#include <QWidget>

class Widget : public QWidget {
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr) : QWidget(parent) {}
};

