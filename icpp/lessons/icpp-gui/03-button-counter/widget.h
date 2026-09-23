// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// ラベルとカウント値を持つ Widget クラスを宣言してください。
// main() は書かず、クリック処理は widget.cpp の lambda connect で実装してください。

#pragma once

#include <QLabel>
#include <QWidget>

class Widget : public QWidget {
public:
    explicit Widget(QWidget *parent = nullptr);

private:
    QLabel *label = nullptr;
    int count = 0;
};
