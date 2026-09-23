#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QTransform>
#include <QDebug>

class Cartesian : public QWidget
{
    Q_OBJECT

public:
    Cartesian();

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent*) override;

private:
    void drawAxis(QPainter* const painter);
};

Cartesian::Cartesian()
    : QWidget(nullptr)
{
}

QSize Cartesian::sizeHint() const
{
    return QSize(400, 400);
}

void Cartesian::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.setWindow(-rect().width()/2, -rect().height()/2, rect().width(), rect().height());

    drawAxis(&painter);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(Qt::blue, 3));
    painter.drawLine(0, 0, 100, 100);

    // Convert from a print coordinate to a Cartesian coordinate.
    QTransform transform;
    transform.scale(1, -1);
    painter.setTransform(transform);

    painter.setPen(QPen(Qt::red, 3, Qt::DotLine));
    painter.drawLine(0, 0, 100, 100);
}

void Cartesian::drawAxis(QPainter* const painter)
{
    painter->drawLine(-rect().width()/2, 0, rect().width(), 0);
    painter->drawLine(0, -rect().height()/2, 0, rect().height());
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Cartesian cartesian;
    cartesian.show();

    app.exec();
}

#include "main.moc"
