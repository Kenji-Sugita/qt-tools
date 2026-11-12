// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// autogen で更新される Qt リソースのメッセージを表示する Widget クラスを宣言してください。
// main() は書かず、loadMessage() を private 関数にしてください。

#pragma once

#include <QWidget>

class QLabel;

class Widget : public QWidget {
public:
    explicit Widget(QWidget *parent = nullptr);

private:
    QString loadMessage() const;

    QLabel *label = nullptr;
};
