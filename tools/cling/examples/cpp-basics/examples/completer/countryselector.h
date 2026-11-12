#ifndef COUNTRYSELECTOR_H
#define COUNTRYSELECTOR_H

#include <QWidget>

class QCompleter;
class QComboBox;

class CountrySelector : public QWidget
{
    Q_OBJECT

public:
    explicit CountrySelector(QWidget* parent = nullptr);

protected slots:
    void changeComlpetionMode(int index) const;
    void changeCaseSensitivity(int index) const;

protected:
    QStringList countries() const;

private:
    QComboBox* completionModeSelector;
    QComboBox* caseSensitivitySelector;
    QCompleter* countryCompleter;
};
#endif
