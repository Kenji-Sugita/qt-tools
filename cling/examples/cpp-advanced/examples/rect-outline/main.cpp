#include <QApplication>
#include <QWidget>
#include <QPainter>

class RectOutline : public QWidget
{
    Q_OBJECT

public:
    explicit RectOutline(QWidget* parent = nullptr)
        : QWidget(parent) {
    }

    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setBrush(Qt::red);

        painter.drawRect(QRectF(2.0, 2.0, 10.0, 10.0));
        painter.fillRect(2, 15, 10, 10, Qt::black);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawRect(QRectF(2.0, 30.0, 10.0, 10.0));
    }
};


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    RectOutline rectOutline;
    rectOutline.resize(100, 100);
    rectOutline.show();

    return app.exec();
}

#include "main.moc"
