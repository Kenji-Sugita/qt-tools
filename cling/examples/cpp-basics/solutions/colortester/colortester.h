#ifndef COLORTESTER_H
#define COLORTESTER_H

#include <QWidget>
class QLabel;

class ColorTester : public QWidget
{
    Q_OBJECT

public:
    explicit ColorTester(QWidget* parent = nullptr);

private slots:
    void slotSelectColor();

private:
    QLabel* const colorLabel;
};
#endif
