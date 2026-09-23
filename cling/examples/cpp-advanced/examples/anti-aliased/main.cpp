#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

class Anitialiasing : public QWidget {
    Q_OBJECT

public:
    explicit Anitialiasing(QWidget* parent = nullptr)
        : QWidget(parent)
    {
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);

        painter.fillRect(rect(), Qt::white);

        QPen pen;
        pen.setWidth(3);
        painter.setPen(pen);

        painter.drawLine(0, 10, width() - 10, height());

        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawLine(10, 0, width(), height() - 10);
    }
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Anitialiasing anitialiasing;
    anitialiasing.resize(400, 350);
    anitialiasing.show();

    return app.exec();
}

#include "main.moc"
