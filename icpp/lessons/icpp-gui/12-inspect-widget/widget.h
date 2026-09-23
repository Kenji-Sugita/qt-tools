// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// PropertyEditor で調べやすい objectName 付き Widget クラスを宣言してください。
// main() は書かず、updatePreview() を private 関数にしてください。

#pragma once

#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QWidget>

class Widget : public QWidget {
public:
    explicit Widget(QWidget *parent = nullptr);

private:
    void updatePreview();

    QLabel *titleLabel = nullptr;
    QLabel *previewLabel = nullptr;
    QSlider *sizeSlider = nullptr;
    QPushButton *accentButton = nullptr;
    bool accent = false;
};
