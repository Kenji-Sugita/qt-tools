#ifndef COMPASSWIDGET_H
#define COMPASSWIDGET_H

#include "direction.h"
#include <QPushButton>
#include <QButtonGroup>

class CompassWidget : public QWidget
{
    Q_OBJECT

public:

    explicit CompassWidget(Compass::Direction direction, QWidget* parent = nullptr);
    explicit CompassWidget(QWidget* parent = nullptr);

    Compass::Direction direction() const;

    bool eventFilter(QObject* receiver, QEvent* event);

public slots:
    void setDirection(Compass::Direction direction);

signals:
    void directionChanged(Compass::Direction direction);

private slots:
    void slotButtonChecked(QAbstractButton* button);

private:
    void checkDirection(Compass::Direction direction);
    Compass::Direction _direction;
    QPushButton* _northButton;
    QPushButton* _northwestButton;
    QPushButton* _northeastButton;
    QPushButton* _westButton;
    QPushButton* _eastButton;
    QPushButton* _southwestButton;
    QPushButton* _southeastButton;
    QPushButton* _southButton;
    QButtonGroup* _buttonGroup;
};

#endif


