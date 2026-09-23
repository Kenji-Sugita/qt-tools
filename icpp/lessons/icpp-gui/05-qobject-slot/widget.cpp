// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QPushButton の clicked を CounterWidget::increment スロットに接続してください。
// go() 関数と moc_widget.cpp のインクルードを用意し、main() は書かないでください。

#include "widget.h"

#include <QPushButton>
#include <QVBoxLayout>

CounterWidget::CounterWidget(QWidget *parent) : QWidget(parent) {
    setWindowTitle("05 Q_OBJECT Slot");

    label = new QLabel("Count: 0", this);
    label->setAlignment(Qt::AlignCenter);

    auto *button = new QPushButton("Count up", this);
    connect(button, &QPushButton::clicked, this, &CounterWidget::increment);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(button, 0, Qt::AlignCenter);

    resize(280, 140);
}

void CounterWidget::increment() {
    ++count;
    label->setText(QString("Count: %1").arg(count));
}

CounterWidget *go() {
    return new CounterWidget;
}

#include "moc_widget.cpp"
