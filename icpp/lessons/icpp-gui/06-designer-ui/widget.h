// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// Designer の form.ui で作ったラベル/ボタン/スライダーを扱う Widget クラスを宣言してください。
// main() は書かず、実装では ui_form.h を使ってください。

#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QSlider;

class Widget : public QWidget {
public:
    explicit Widget(QWidget *parent = nullptr);

private:
    QLabel *label = nullptr;
    QPushButton *button = nullptr;
    QSlider *slider = nullptr;
};
