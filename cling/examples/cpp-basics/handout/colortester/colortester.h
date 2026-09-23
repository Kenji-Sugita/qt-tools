#ifndef COLORTESTER_H
#define COLORTESTER_H

#include <QWidget>
class QLabel;

class ColorTester : public QWidget
{
public:
    explicit ColorTester(QWidget* parent = nullptr);

private:
    QLabel* const colorLabel;
};
#endif
