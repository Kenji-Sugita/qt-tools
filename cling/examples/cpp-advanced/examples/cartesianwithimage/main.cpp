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
    QSize calculateSizeHint() const;
    QPixmap pixmap;
};

Cartesian::Cartesian()
    : QWidget(nullptr)
{
    QFont largeFont = font();
    largeFont.setPointSize(32);
    setFont(largeFont);

    pixmap.load(":/violet-necked-lory.png");

    setFixedSize(calculateSizeHint());
}

QSize Cartesian::calculateSizeHint() const
{
    static QSize size;
    if (size.isEmpty()) {
        const int maxSide = qMax(pixmap.size().width(), pixmap.size().height());
        size = QSize(2*maxSide, 2*maxSide);
    }
    return size;
}

QSize Cartesian::sizeHint() const
{
    return calculateSizeHint();
}

void Cartesian::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.setWindow(-rect().width()/2, -rect().height()/2, rect().width(), rect().height());

    drawAxis(&painter);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(Qt::blue, 3));
    painter.drawLine(0, 0, rect().width()/2, rect().height()/2);

    // Convert to Cartesian coordinate.
    QTransform transform = painter.transform();
    transform.scale(1, -1);
    painter.setTransform(transform);

    painter.drawPixmap(0, 0, pixmap);

    painter.setPen(QPen(Qt::red, 3, Qt::DotLine));
    painter.drawLine(0, 0, rect().width()/2, rect().height()/2);

    painter.setPen(QPen(Qt::white, 3));
    painter.translate(0, painter.fontMetrics().descent());
    painter.drawText(0, 0, "Violet-necked Lory");
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
