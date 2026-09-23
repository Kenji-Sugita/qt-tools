#include <QApplication>
#include <QWidget>
#include <QPaintEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QLayout>
#include <QPainter>
#include <QDebug>

#define USE_COORDINATE_MODE_OBJECT_BOUNDING

class Gradient : public QWidget {
    Q_OBJECT

public:
    explicit Gradient(QWidget* parent = nullptr)
        : QWidget(parent)
    {
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const int w = width();
        const int h = static_cast<int>(height()/3.0);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        {
#if defined(USE_COORDINATE_MODE_OBJECT_BOUNDING)
            QLinearGradient gradient;
            gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
            // gradient.start();      // (0, 0)
            // gradient.finalStop();  // (1, 1)
#else
            QLinearGradient gradient;
            gradient.setStart(5, 5);
            gradient.setFinalStop(w - 10, h - 10);
#endif
            gradient.setColorAt(0.0, Qt::red);
            gradient.setColorAt(0.5, Qt::green);
            gradient.setColorAt(1.0, Qt::blue);
            painter.setBrush(gradient);
            painter.drawRect(5, 5, w - 10, h - 10);
        }

        {
#if defined(USE_COORDINATE_MODE_OBJECT_BOUNDING)
            QRadialGradient gradient;
            gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
            gradient.setCenter(1.0/2.0/0.9, 1.0/2.0/0.9);
            gradient.setRadius((1.0/2.0) * (10.0/9.0) );
            gradient.setFocalPoint((w/3.0 - 5) / (w - 10.0), (h/3.0 - 5) / (h - 10.0));
#else
            QRadialGradient gradient;
            gradient.setCenter(w/2.0 + 5, h + h/2.0 + 5);
            gradient.setRadius(w/2.0);
            gradient.setFocalPoint(w/3.0, h + h/3.0);
#endif
            gradient.setColorAt(0.0, Qt::white);
            gradient.setColorAt(0.5, Qt::green);
            gradient.setColorAt(1.0, Qt::black);
            painter.setBrush(gradient);
            painter.drawEllipse(5, h + 5, w - 10, h - 10);
        }

        {
#if defined(USE_COORDINATE_MODE_OBJECT_BOUNDING)
            QConicalGradient gradient;
            gradient.setCenter(((w * 2.0/3.0) - 5) / (w - 10), ((h * 1.0/3.0) - 5) / (h - 10));
            gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
#else
            QConicalGradient gradient;
            gradient.setCenter(2 * w/3.0, 2 * h + h/3.0);
            gradient.setAngle(0);
#endif
            gradient.setColorAt(0.0, Qt::black);
            gradient.setColorAt(0.4, Qt::green);
            gradient.setColorAt(0.6, Qt::white);
            gradient.setColorAt(1.0, Qt::black);
            painter.setBrush(gradient);
            painter.drawEllipse(5, 2 * h + 5, w - 10, h - 10);
        }
    }
};


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Gradient gradient;
    gradient.resize(100, 300);
    gradient.show();

    return app.exec();
}

#include "main.moc"
