#ifndef COMPASSWIDGET2_H
#define COMPASSWIDGET2_H

#include "direction.h"
#include <QWidget>
class QPolygon;
class QRegion;

class CompassWidget2 : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Compass::Direction direction READ direction WRITE setDirection)

public:
    explicit CompassWidget2(Compass::Direction direction, QWidget* parent = nullptr);
    explicit CompassWidget2(QWidget* parent = nullptr);

    Compass::Direction direction() const;

    virtual QSize sizeHint() const override { return QSize(100, 100); }

public slots:
    void setDirection(Compass::Direction direction);

signals:
    void directionChanged(Compass::Direction direction);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private:
    void setDirectionAndEmit(Compass::Direction direction);
    Compass::Direction _direction;
    QPolygon _points[Compass::NumDirections];
    QRegion _region[Compass::NumDirections];
};
#endif


