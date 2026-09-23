#include <QApplication>
#include <QWidget>
#include <QPixmap>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QDebug>

class Transparency : public QWidget
{
    Q_OBJECT

public:
    explicit Transparency(QWidget* parent = nullptr)
        : QWidget(parent), count(0)
    {
        const auto timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Transparency::next);
        QTimer::singleShot(0, [timer]() {
                timer->start(500);
            });

        // Background image
        const QPixmap pixmap(":/background.png");
        setFixedSize(pixmap.size());
        QPalette backgroundPalette = palette();
        backgroundPalette.setBrush(QPalette::Window, pixmap);
        setPalette(backgroundPalette);

        setFont(QFont(font().family(), fontSize));
    }

protected slots:
    void next()
    {
        updateRects(count);
        count = (count + 1) % maxCount;
        updateRects(count);

        // Update the bounding box of the text
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        const int textWidth = fontMetrics().horizontalAdvance(message);
#else
        const int textWidth = fontMetrics().boundingRect(message).width();
#endif
        const int textHeight = fontMetrics().height();
        const QRect textRect((rect().width() - textWidth)/2.0, startX - fontMetrics().ascent(),
                              textWidth, textHeight);
        update(textRect);
    }

private:
    void updateRects(int count) {
        const int x = startX + (sideLength + gapBetweenSquares)*(count % maxColumns);
        const int y = startY + (sideLength + gapBetweenSquares)*(count / maxColumns);
        update(QRect(x, y, sideLength, sideLength).adjusted(-1, -1, 1, 1));
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);

        // Drawing Text
        QColor color = Qt::yellow;
        color.setAlpha((256 * count) / maxCount);
        painter.setPen(color);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        painter.drawText((rect().width() - fontMetrics().horizontalAdvance(message)) / 2.0, startX, message);
#else
        painter.drawText((rect().width() - fontMetrics().boundingRect(message).width()) / 2.0, startX, message);
#endif

        // Drawing all rectangles
        for (int i = 0; i < maxCount; ++i) {
            color.setAlpha((256 * i)/maxCount);
            painter.setPen(i == count ? Qt::black : color);
            painter.setBrush(color);
            painter.drawRect(startX + (sideLength + gapBetweenSquares)*(i % maxColumns),
                             startY + (sideLength + gapBetweenSquares)*(i / maxColumns),
                             sideLength,
                             sideLength);
        }
    }

private:
    int count;
    const int fontSize = 130;
    const int startX = 150;
    const int startY = 300;
    const int sideLength = 100;
    const int gapBetweenSquares = 10;
    const int maxCount = 10;
    const int maxColumns = 5;
    const QString message = "Hello World";
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Transparency transparency;
    transparency.show();

    return app.exec();
}

#include "main.moc"
