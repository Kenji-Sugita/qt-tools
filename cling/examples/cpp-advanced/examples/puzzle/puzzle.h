#ifndef PUZZLE_H
#define PUZZLE_H

#include <QWidget>
#include <QPixmap>
#include <QRegion>
class QMouseEvent;
class QPaintEvent;

class Puzzle :public QWidget
{
    Q_OBJECT

public:
    Puzzle(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;

public slots:
    void showNext();

private:
    QPixmap image;
    QPixmap question;
    QVector<QRegion> regions;
    int count;
};
#endif
