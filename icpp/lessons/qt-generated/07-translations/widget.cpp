// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// qrc_translations.cpp を include し、QTranslator で app_ja.qm の翻訳を QLabel に表示してください。
// go() 関数を用意し、main() は書かないでください。

#include <QCoreApplication>
#include <QLabel>
#include <QString>
#include <QTranslator>
#include <QVBoxLayout>
#include <QWidget>

#include "qrc_translations.cpp"

QWidget *go() {
    QTranslator translator;
    if (!translator.load(":/i18n/app_ja.qm")) {
        auto *widget = new QWidget;
        widget->setWindowTitle("07 Translations");
        auto *label = new QLabel("translation load failed", widget);
        auto *layout = new QVBoxLayout(widget);
        layout->addWidget(label);
        widget->resize(240, 100);
        return widget;
    }
    QCoreApplication::installTranslator(&translator);
    const QString text = QCoreApplication::translate("IcppVerification", "Hello");
    QCoreApplication::removeTranslator(&translator);

    auto *widget = new QWidget;
    widget->setWindowTitle("07 Translations");
    auto *label = new QLabel(text, widget);
    auto *layout = new QVBoxLayout(widget);
    layout->addWidget(label);
    widget->resize(240, 100);
    return widget;
}
