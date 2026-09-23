#include "countryselector.h"

#include <QGridLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QComboBox>
#include <QCompleter>
#include <QCheckBox>
#include <QLineEdit>
#include <QFile>
#include <QTextStream>
#include <QDebug>

Q_DECLARE_METATYPE(QCompleter::CompletionMode);

CountrySelector::CountrySelector(QWidget* parent )
    : QWidget(parent)
{
    const auto topLayout = new QGridLayout(this);
    topLayout->setColumnStretch(1, 1);
    int row = -1;

    // Completion Mode
    completionModeSelector = new QComboBox;
    completionModeSelector->addItem("Filtered Popup Completion", QCompleter::PopupCompletion);
    completionModeSelector->addItem("Unfiltered Popup Completion", QCompleter::UnfilteredPopupCompletion);
    completionModeSelector->addItem("Inline Completion", QCompleter::InlineCompletion);

    topLayout->addWidget(new QLabel("Completion Mode"), ++row, 0);
    topLayout->addWidget(completionModeSelector, row, 1, Qt::AlignLeft);
    connect(completionModeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CountrySelector::changeComlpetionMode);

    // Case Sensitivity
    caseSensitivitySelector = new QComboBox;
    caseSensitivitySelector->addItem("Case Sensitive", Qt::CaseSensitive);
    caseSensitivitySelector->addItem("Case Insensitive", Qt::CaseInsensitive);
    connect(caseSensitivitySelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CountrySelector::changeCaseSensitivity);

    topLayout->addWidget(new QLabel("Case Sensitivity"), ++row, 0);
    topLayout->addWidget(caseSensitivitySelector, row, 1, Qt::AlignLeft);

    // Wrap around completions
    const auto wrapAroundCompletionsSelector = new QCheckBox("Wrap arround completions");
    wrapAroundCompletionsSelector->setChecked(true);
    topLayout->addWidget(wrapAroundCompletionsSelector, ++row, 0, 1, 2);

    // Country Edit
    const auto countryEdit = new QLineEdit;
    countryCompleter = new QCompleter(countries(), this);
    countryEdit->setCompleter(countryCompleter);

    topLayout->addWidget(new QLabel("Country"), ++row, 0);
    topLayout->addWidget(countryEdit, row, 1);

    topLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), ++row, 0);

    // Should call setFocus() after layouting.
    countryEdit->setFocus();

    connect(wrapAroundCompletionsSelector, &QCheckBox::clicked, countryCompleter, &QCompleter::setWrapAround);
}

void CountrySelector::changeComlpetionMode(int index) const
{
    countryCompleter->setCompletionMode(completionModeSelector->itemData(index).value<QCompleter::CompletionMode>());
}

QStringList CountrySelector::countries() const
{
    static QStringList countries;

    if (countries.count() == 0) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        for (int territoryIndex = QLocale::AnyTerritory + 1; territoryIndex <= QLocale::LastTerritory; ++territoryIndex) {
            countries << QLocale::territoryToString(static_cast<QLocale::Country>(territoryIndex));
        }
#else			 
        for (int countryIndex = QLocale::AnyCountry + 1; countryIndex <= QLocale::LastCountry; ++countryIndex) {
            countries << QLocale::countryToString(static_cast<QLocale::Country>(countryIndex));
        }
#endif
        countries.sort();
    }

    return countries;
}

void CountrySelector::changeCaseSensitivity(int index) const
{
    countryCompleter->setCaseSensitivity(caseSensitivitySelector->itemData(index).value<Qt::CaseSensitivity>());
}
