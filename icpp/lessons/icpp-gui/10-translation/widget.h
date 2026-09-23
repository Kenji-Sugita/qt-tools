// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// tr() で翻訳済みテキストを表示する Widget クラスを宣言してください。

#pragma once

#include <QWidget>

class QLabel;

class Widget : public QWidget {
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);

private:
    QLabel *label = nullptr;
};
