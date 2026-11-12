#include <QApplication>
#include <QLCDNumber>
#include <QSlider>
#include <QLineEdit>
#include <QVBoxLayout>

#include "lcdslider.h"

//
// QLCDNumber の表示を確認するためのサンプルコード
//
LCDSlider::LCDSlider(QWidget* parent) : QWidget(parent)
{
    auto layout = new QVBoxLayout{this};
    auto lcd = new QLCDNumber{};
    auto slider = new QSlider{Qt::Horizontal};
    auto lineEdit = new QLineEdit{};

    connect(slider, &QSlider::valueChanged, lcd, qOverload<int>(&QLCDNumber::display));

    connect(lineEdit, &QLineEdit::textChanged, [lcd](const QString& text) {
        lcd->display(text);
    });

    layout->addWidget(lcd);
    layout->addWidget(slider);
    layout->addWidget(lineEdit);
}

//
// Cling 用のコード
//
#if defined(__CLING__)
#include "moc_lcdslider.cpp"
auto w = new LCDSlider{};
w->show();
auto l = w->findChild<QVBoxLayout *>();
auto lcd = w->findChild<QLCDNumber *>();
auto slider = w->findChild<QSlider *>();
auto edit = w->findChild<QLineEdit *>();
#endif









