#ifndef HARNESS_H
#define HARNESS_H

#include <QWidget>

class QComboBox;
class QLabel;

class Harness : public QWidget
{
    Q_OBJECT

public:
    static constexpr const char* translatorTypeProperty = "TranslatorType";
    static constexpr const char* applicationTranslatorName = "ApplicationTranslator";

    static const QStringList translationFileSuffixes;
    static const QString translationFileBaseName;

public:
    Harness();

protected:
    void changeEvent(QEvent* event) override;

protected slots:
    void changeLanguage(int index);

private:
    void retranslateStrings();

    QComboBox* languageSelector = nullptr;
    QLabel* messageLabel = nullptr;
};
#endif
