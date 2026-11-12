// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// autogen 後の qrc_resources.cpp をインクルードし、リソースの文字列を表示してください。
// loadMessage() と go() 関数を用意し、main() は書かないでください。

#include "widget.h"

#include <QFile>
#include <QLabel>
#include <QTextStream>
#include <QVBoxLayout>

#include "qrc_resources.cpp"

QString Widget::loadMessage() const {
    QFile file(":/data/message.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "resource open failed";
    }
    QTextStream stream(&file);
    return stream.readAll().trimmed();
}

Widget::Widget(QWidget *parent) : QWidget(parent) {
    setWindowTitle("09 Autogen");
    label = new QLabel(loadMessage(), this);
    label->setAlignment(Qt::AlignCenter);
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    resize(340, 120);
}

Widget *go() {
    return new Widget;
}
