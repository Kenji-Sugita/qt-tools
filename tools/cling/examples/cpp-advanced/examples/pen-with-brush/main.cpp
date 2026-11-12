#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QFont>
#include <QBrush>
#include <QPen>
#include <QDebug>

class PenWithBrush : public QWidget
{
    Q_OBJECT

public:
    explicit PenWithBrush(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setFont(QFont(fontFamilyName, fontPixelSize));
        setFixedSize(fontMetrics().horizontalAdvance(message), fontMetrics().height());
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);

        QLinearGradient gradient(0, 0.0, 1.0, 0.0);  // Left to right
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        gradient.setColorAt(0, Qt::red);
        gradient.setColorAt(0.5, Qt::green);
        gradient.setColorAt(1, Qt::blue);

        const QPen pen(gradient, Qt::SolidLine);
        painter.setPen(pen);

        painter.drawText(0, fontMetrics().ascent(), message);
    }

private:
    const QString fontFamilyName = "Helvetica";
    const QString message = "Hello World";
    const int fontPixelSize = 100;
};


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    PenWithBrush penWithBrush;
    penWithBrush.show();

    return app.exec();
}

#include "main.moc"
