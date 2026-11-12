#include "puzzle.h"
#include <QPainter>
#include <QPaintEvent>

Puzzle::Puzzle(QWidget* parent)
    :QWidget(parent), count(0)
{
    image = QPixmap(":/Images/jesper.jpg");
    question = QPixmap(":/Images/question.png");

    const QPolygon bodyPoints({ QPoint(50, 180), QPoint(310, 260), QPoint(420, 330), QPoint(520, 560),
                                QPoint(360 , 800), QPoint(200, 800), QPoint(50, 400) });

    const QPolygon eyePoints({ QPoint(360, 220), QPoint(430, 225), QPoint(420, 255), QPoint(350, 240) });

    regions << QRegion(QRect(QPoint(50, 150), QPoint(170, 300)), QRegion::Ellipse)
            << QRect(QPoint(180, 350), QPoint(380, 650))
            << bodyPoints
            << eyePoints
            << QRect(QPoint(340, 100), QPoint(460, 180))
            << QRect(0,0, image.width(), image.height());

    setFixedSize(image.size());
}

void Puzzle::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    painter.drawPixmap(0, 0, question);
    QRegion region;
    for (int i = 0; i < count; ++i) {
        region += regions.at(i);
    }
    painter.setClipRegion(region);
    painter.drawPixmap(0, 0, image);

#if 0
    // Showing updating area.
    for (const auto& rect : event->region()) {
        painter.setPen(Qt::red);
        painter.drawRect(rect);
    }
#endif
}

void Puzzle::showNext()
{
    count = (count + 1) % (regions.size() + 1);
    if (count > 0) {
        update(regions.at(count - 1));  // Minimum updating area.
    } else {
        update();
    }
}
