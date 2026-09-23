#ifndef MOVING_H
#define MOVING_H

#include <QWidget>

class Moving : public QWidget
{
    Q_OBJECT

public:
    explicit Moving(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

public slots:
    void move();
    void reset();

private:
    const int sideLength = 40;
    QPoint startPosition;
    QPoint endPosition;
    QPoint currentPosition;
};
#endif
