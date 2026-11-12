// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QSlider と QLabel を持ち、値表示を更新する Widget クラスを宣言してください。
// main() は書かず、状態更新用の private 関数を持たせてください。

#pragma once

#include <QLabel>
#include <QSlider>
#include <QWidget>

class Widget : public QWidget {
public:
    explicit Widget(QWidget *parent = nullptr);

private:
    void updateLabel(int value);

    QLabel *label = nullptr;
    QSlider *slider = nullptr;
};
