#include "harness.h"

#include <QApplication>
#include <QTranslator>
#include <QDebug>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    const auto translator = new QTranslator(&app);
    if (!translator->load(Harness::translationFileBaseName + Harness::translationFileSuffixes.last())) {  // The suffix of Japanese
        qWarning() << "Cannot load the translation.";
    }
    translator->setProperty(Harness::translatorTypeProperty, Harness::applicationTranslatorName);

    QApplication::installTranslator(translator);

    Harness harness;
    harness.show();

    return app.exec();
}
