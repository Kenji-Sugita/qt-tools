#include <QCoreApplication>
#include <QString>
#include <QTranslator>

#include "qrc_translations.cpp"

QString translatedText() {
    QTranslator translator;
    if (!translator.load(":/i18n/app_ja.qm")) {
        return "translation load failed";
    }
    QCoreApplication::installTranslator(&translator);
    const QString text = QCoreApplication::translate("IcppVerification", "Hello");
    QCoreApplication::removeTranslator(&translator);
    return text;
}
