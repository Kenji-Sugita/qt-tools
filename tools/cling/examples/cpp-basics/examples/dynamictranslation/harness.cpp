#include "harness.h"
#include <QApplication>
#include <QTranslator>
#include <QLabel>
#include <QComboBox>
#include <QStandardItemModel>
#include <QEvent>
#include <QLayout>
#include <QDebug>

#define MAIN_TRANSLATION_CONTEXT "Main"

static const char* const languageKeys[] = {
    QT_TRANSLATE_NOOP("Main", "English"),
    QT_TRANSLATE_NOOP("Main", "Japanese")
};
const QStringList Harness::translationFileSuffixes = { "_en", "_ja" };
const QString Harness::translationFileBaseName = ":/dynamictranslation";


Harness::Harness()
    : QWidget(nullptr)
{
    languageSelector = new QComboBox;
    for (const char* key : languageKeys) {
        languageSelector->addItem(QString::fromLatin1(key));
    }
    languageSelector->setCurrentIndex(1);

    messageLabel = new QLabel;

    const auto languageSelectorLayout = new QHBoxLayout;
    languageSelectorLayout->addWidget(languageSelector);
    languageSelectorLayout->addStretch();

    const auto topLayout = new QVBoxLayout(this);
    topLayout->addLayout(languageSelectorLayout);
    topLayout->addWidget(messageLabel);
    topLayout->addStretch();

    retranslateStrings();

    connect(languageSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Harness::changeLanguage);
}

void Harness::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        QWidget::changeEvent(event);
        retranslateStrings();
        return;
    }
    QWidget::changeEvent(event);
}

void Harness::changeLanguage(int index)
{
    for (const auto translator : QApplication::instance()->findChildren<QTranslator*>()) {
        if (translator->property(translatorTypeProperty).toString() == Harness::applicationTranslatorName) {
            QApplication::removeTranslator(translator);
            if (!translator->load(translationFileBaseName + translationFileSuffixes.at(index))) {
                qWarning() << "Cannot load the translation.";
            }
            QApplication::installTranslator(translator);
            break;
        }
    }
}

void Harness::retranslateStrings()
{
    for (int i = 0; i < languageSelector->count(); i++) {
        languageSelector->setItemText(i, QCoreApplication::translate("Main", languageKeys[i]));
    }

    messageLabel->setText(tr("Universe"));
}
