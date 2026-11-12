#include "compasswidget.h"
#include <QGridLayout>
#include <QKeyEvent>
#include <QDebug>

CompassWidget::CompassWidget(Compass::Direction direction, QWidget* parent) 
    : QWidget(parent)
{
    _buttonGroup = new QButtonGroup(this);
    _buttonGroup->setExclusive(true);
    connect(_buttonGroup, static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked), this, &CompassWidget::slotButtonChecked);

    QGridLayout* layout = new QGridLayout(this);

    // row 1
    _northButton = new QPushButton(tr("N"), this);
    _northButton->setCheckable(true);
    layout->addWidget(_northButton, 0, 2);
    _buttonGroup->addButton(_northButton);

    // row 2
    _northwestButton = new QPushButton(tr("NW"), this);
    _northwestButton->setCheckable(true);
    layout->addWidget(_northwestButton, 1, 1);
    _buttonGroup->addButton(_northwestButton);
    _northeastButton = new QPushButton(tr("NE"), this);
    _northeastButton->setCheckable(true);
    layout->addWidget(_northeastButton, 1, 3);
    _buttonGroup->addButton(_northeastButton);

    // row 3
    _westButton = new QPushButton(tr("W"), this);
    _westButton->setCheckable(true);
    layout->addWidget(_westButton, 2, 0);
    _buttonGroup->addButton(_westButton);
    _eastButton = new QPushButton(tr("E"), this);
    _eastButton->setCheckable(true);
    layout->addWidget(_eastButton, 2, 4);
    _buttonGroup->addButton(_eastButton);

    // row 4
    _southwestButton = new QPushButton(tr("SW"), this);
    _southwestButton->setCheckable(true);
    layout->addWidget(_southwestButton, 3, 1);
    _buttonGroup->addButton(_southwestButton);
    _southeastButton = new QPushButton(tr("SE"), this);
    _southeastButton->setCheckable(true);
    layout->addWidget(_southeastButton, 3, 3);
    _buttonGroup->addButton(_southeastButton);

    // row 5
    _southButton = new QPushButton(tr("S"), this);
    _southButton->setCheckable(true);
    layout->addWidget(_southButton, 4, 2);
    _buttonGroup->addButton(_southButton);

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setFocusPolicy(Qt::NoFocus);
    }

    _direction = direction;
    checkDirection(_direction);

    installEventFilter(this);
    setFocusPolicy(Qt::ClickFocus);
}

CompassWidget::CompassWidget(QWidget* parent) 
    : CompassWidget(Compass::North, parent)
{
}

void CompassWidget::setDirection(Compass::Direction direction)
{
    if (_direction == direction)
        return;

    _direction = direction;
    checkDirection(_direction);

    emit directionChanged(_direction);
}

void CompassWidget::checkDirection(Compass::Direction direction)
{
    switch(direction) {
    case Compass::North:
        _northButton->setChecked(true);
        break;
    case Compass::NorthWest:
        _northwestButton->setChecked(true);
        break;
    case Compass::NorthEast:
        _northeastButton->setChecked(true);
        break;
    case Compass::East:
        _eastButton->setChecked(true);
        break;
    case Compass::West:
        _westButton->setChecked(true);
        break;
    case Compass::SouthWest:
        _southwestButton->setChecked(true);
        break;
    case Compass::SouthEast:
        _southeastButton->setChecked(true);
        break;
    case Compass::South:
        _southButton->setChecked(true);
        break;
    default:
	break;
    }
}

Compass::Direction CompassWidget::direction() const
{
    return _direction;
}

bool CompassWidget::eventFilter(QObject* receiver, QEvent* event)
{
    Q_UNUSED(receiver);

    if (event->type() != QEvent::KeyPress)
        return false;

    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

    switch (keyEvent->key()) {
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

    return false;
}

void CompassWidget::slotButtonChecked(QAbstractButton* button)
{
    Compass::Direction newDirection;

    if (button == _northButton) {
        newDirection = Compass::North;
    } else if (button == _northwestButton) {
        newDirection = Compass::NorthWest;
    } else if (button == _northeastButton) {
        newDirection = Compass::NorthEast;
    } else if (button == _westButton) {
        newDirection = Compass::West;
    } else if (button == _eastButton) {
        newDirection = Compass::East;
    } else if (button == _southwestButton) {
        newDirection = Compass::SouthWest;
    } else if (button == _southeastButton) {
        newDirection = Compass::SouthEast;
    } else {
        newDirection = Compass::South;
    }

    setDirection(newDirection);
}
