#include "compasswidget2.h"
#include <QMouseEvent>
#include <QPainter>

CompassWidget2::CompassWidget2(Compass::Direction direction, QWidget* parent) 
    : QWidget(parent)
{
    _direction = direction;
    setFocusPolicy(Qt::ClickFocus);
    for (int direction = Compass::North; direction < Compass::NumDirections; ++direction) {
        _points[direction].resize(5);
    }
}

CompassWidget2::CompassWidget2(QWidget* parent) 
    : CompassWidget2(Compass::North, parent)
{
}
/*
void CompassWidget2::init()
{
    setFocusPolicy(Qt::ClickFocus);
    for (int direction = Compass::North; direction < Compass::NumDirections; ++direction) {
        _points[direction].resize(5);
    }
}
*/

void CompassWidget2::setDirection(Compass::Direction direction)
{
    if (_direction == direction)
        return;

    update(_region[_direction].boundingRect());
    _direction = direction;
    update(_region[_direction].boundingRect());
    emit directionChanged(direction);
}

Compass::Direction CompassWidget2::direction() const
{
    return _direction;
}

void CompassWidget2::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.setRenderHint(QPainter::Antialiasing);

    QBrush fillBrush(palette().color(QPalette::Dark));

    for (int direction = Compass::North; direction < Compass::NumDirections; ++direction) {
        painter.save();
        if (_direction == direction )
            painter.setBrush(fillBrush);
        painter.drawPolygon(_points[direction]);
        painter.restore();
    }
}

void CompassWidget2::resizeEvent(QResizeEvent*)
{
    int width_1_8 = width() / 8;
    int width_3_16 = width() * 3 / 16;
    int width_3_8 = width() * 3 / 8;
    int width_7_16 = width() * 7 / 16;
    int width_1_2 = width() / 2;
    int width_9_16 = width() * 9 / 16;
    int width_5_8 = width() * 5 / 8;
    int width_13_16 = width() * 13 / 16;
    int width_7_8 = width() * 7 / 8;
    int height_1_8 = height() / 8;
    int height_3_16 = height() * 3 / 16;
    int height_3_8 = height() * 3 / 8;
    int height_7_16 = height() * 7 / 16;
    int height_1_2 = height() / 2;
    int height_9_16 = height() * 9 / 16;
    int height_5_8 = height() * 5 / 8;
    int height_13_16 = height() * 13 / 16;
    int height_7_8 = height() * 7 / 8;

    _points[Compass::North].setPoint(0, width_1_2, 0);
    _points[Compass::North].setPoint(1, width_3_8, height_3_8);
    _points[Compass::North].setPoint(2, width_1_2, height_1_2);
    _points[Compass::North].setPoint(3, width_5_8, height_3_8);
    _points[Compass::North].setPoint(4, width_1_2, 0);

    _points[Compass::NorthWest].setPoint(0, width_1_8, height_1_8);
    _points[Compass::NorthWest].setPoint(1, width_3_16, height_7_16);
    _points[Compass::NorthWest].setPoint(2, width_3_8, height_3_8);
    _points[Compass::NorthWest].setPoint(3, width_7_16, height_3_16);
    _points[Compass::NorthWest].setPoint(4, width_1_8, height_1_8);

    _points[Compass::NorthEast].setPoint(0, width_7_8, height_1_8);
    _points[Compass::NorthEast].setPoint(1, width_13_16, height_7_16);
    _points[Compass::NorthEast].setPoint(2, width_5_8, height_3_8);
    _points[Compass::NorthEast].setPoint(3, width_9_16, height_3_16);
    _points[Compass::NorthEast].setPoint(4, width_7_8, height_1_8);

    _points[Compass::West].setPoint(0, 0, height_1_2);
    _points[Compass::West].setPoint(1, width_3_8, height_3_8);
    _points[Compass::West].setPoint(2, width_1_2, height_1_2);
    _points[Compass::West].setPoint(3, width_3_8, height_5_8);
    _points[Compass::West].setPoint(4, 0, height_1_2);

    _points[Compass::East].setPoint(0, width(), height_1_2);
    _points[Compass::East].setPoint(1, width_5_8, height_3_8);
    _points[Compass::East].setPoint(2, width_1_2, height_1_2);
    _points[Compass::East].setPoint(3, width_5_8, height_5_8);
    _points[Compass::East].setPoint(4, width(), height_1_2);

    _points[Compass::SouthWest].setPoint(0, width_1_8, height_7_8);
    _points[Compass::SouthWest].setPoint(1, width_3_16, height_9_16);
    _points[Compass::SouthWest].setPoint(2, width_3_8, height_5_8);
    _points[Compass::SouthWest].setPoint(3, width_7_16, height_13_16);
    _points[Compass::SouthWest].setPoint(4, width_1_8, height_7_8);

    _points[Compass::SouthEast].setPoint(0, width_7_8, height_7_8);
    _points[Compass::SouthEast].setPoint(1, width_13_16, height_9_16);
    _points[Compass::SouthEast].setPoint(2, width_5_8, height_5_8);
    _points[Compass::SouthEast].setPoint(3, width_9_16, height_13_16);
    _points[Compass::SouthEast].setPoint(4, width_7_8, height_7_8);

    _points[Compass::South].setPoint(0, width_1_2, height());
    _points[Compass::South].setPoint(1, width_3_8, height_5_8);
    _points[Compass::South].setPoint(2, width_1_2, height_1_2);
    _points[Compass::South].setPoint(3, width_5_8, height_5_8);
    _points[Compass::South].setPoint(4, width_1_2, height());

    for (int direction = Compass::North; direction < Compass::NumDirections; ++direction) {
        _region[direction] = QRegion(_points[direction]);
    }

}

void CompassWidget2::mousePressEvent(QMouseEvent* event)
{
    for (int direction = Compass::North; direction < Compass::NumDirections; ++direction) {
        if (_region[direction].contains(event->pos())) {
            setDirection(static_cast<Compass::Direction>(direction));
            break;
        }
    }
}

void CompassWidget2::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Right:
        if (_direction == Compass::North)
            setDirection(Compass::NorthEast);
        else
            setDirection(static_cast<Compass::Direction>(_direction - 1));
	break;
    case Qt::Key_Left:
        if (_direction == Compass::NorthEast)
            setDirection(Compass::North);
        else
            setDirection(static_cast<Compass::Direction>(_direction + 1));
        break;
    default:
        break;
    }
}
