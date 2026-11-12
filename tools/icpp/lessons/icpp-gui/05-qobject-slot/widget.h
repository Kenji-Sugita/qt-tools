// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// Q_OBJECT と public スロットを持つ CounterWidget クラスを宣言してください。
// main() は書かず、moc 生成が必要な教材として使える形にしてください。

#pragma once

#include <QLabel>
#include <QWidget>

class CounterWidget : public QWidget {
    Q_OBJECT

public:
    explicit CounterWidget(QWidget *parent = nullptr);

public slots:
    void increment();

private:
    QLabel *label = nullptr;
    int count = 0;
};
