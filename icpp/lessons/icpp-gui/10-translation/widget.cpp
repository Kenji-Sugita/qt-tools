// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// qrc_translations.cpp をインクルードし、:/i18n/app_ja.qm から翻訳を読み込んでください。
// go() 関数を用意し、main() は書かないでください。

#include "widget.h"

#include <QCoreApplication>
#include <QLabel>
#include <QTranslator>
#include <QVBoxLayout>

// qrc_translations.cpp を生成したら、
// 以下の行のコメントを外してください。
// #include "qrc_translations.cpp"

Widget::Widget(QWidget *parent) : QWidget(parent) {
    setWindowTitle("10 Translation");

    label = new QLabel(tr("Hello"));
    label->setAlignment(Qt::AlignCenter);
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);

    resize(260, 120);
}

Widget *go() {
    static QTranslator translator;
    if (translator.load(":/i18n/app_ja.qm")) {
        QCoreApplication::installTranslator(&translator);
    }
    return new Widget;
}

#include "moc_widget.cpp"
