#include <QApplication>
#include <QWidget>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPainter>
#include <QWindow>

class Hourses : public QWidget {
    Q_OBJECT

public:
    explicit Hourses(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        largeFont = QFont("Helvetica");
        largeFont.setPixelSize(50);
        largeFontMetrics = QFontMetrics(largeFont);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

        // Thin pen
        Q_ASSERT(windowHandle());
        QPen pen = painter.pen();
        pen.setWidthF(1/windowHandle()->devicePixelRatio());
        painter.setPen(pen);

        painter.fillRect(rect(), Qt::white);

        // Blue house
        {
            QPainterPath house;
            house.addRect(QRect(QPoint(300, 250), QPoint(400, 350)));  // Body
            house.addRect(QRect(QPoint(340, 350), QPoint(360, 300)));  // Door
            house.addRect(QRect(QPoint(310, 270), QPoint(330, 290)));  // Left window
            house.addRect(QRect(QPoint(390, 270), QPoint(370, 290)));  // Right window

            painter.setBrush(Qt::blue);
            painter.fillPath(house, painter.brush());

            painter.setBrush(Qt::yellow);
            QPainterPath roof;
            roof.moveTo(301, 250);  // Bottom left
            roof.lineTo(350, 150);  // Top center
            roof.lineTo(400, 250);  // Bottom right
            roof.closeSubpath();
            painter.drawPath(roof);
        }

        // Green houses
        {
            QPainterPath house;
            house.addRect(QRect(QPoint(340, 350), QPoint(360, 300)));  // Door
            house.addRect(QRect(QPoint(310, 270), QPoint(330, 290)));  // Left window
            house.addRect(QRect(QPoint(390, 270), QPoint(370, 290)));  // Right window
            house.moveTo(300, 250);                                    // Top left of body
            house.lineTo(300, 350);                                    // Bottom left of body
            house.lineTo(400, 350);                                    // Bottom right of body
            house.lineTo(400, 250);                                    // Top right of body
            house.quadTo(QPoint(350, 150), QPoint(300, 250));          // Roof

            painter.setBrush(Qt::green);
            for (int i = -2; i <= 2; ++i) {
                if (i != 0) {
                    painter.save();
                    painter.translate(120 * i, 0);
                    if (abs(i) == 1) {
                        painter.drawPath(house);
                    } else {
                        painter.strokePath(house, painter.pen());
                    }
                    painter.restore();
                }
            }
        }

        // Text
        QPainterPath textPath;
        textPath.addText(50, 80, largeFont, message);

        QLinearGradient gradient(0.0, 0.0, 1.0, 0.0);  // Left to right
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        gradient.setColorAt(0, QColor(Qt::blue).darker());
        gradient.setColorAt(0.5, "green");
        gradient.setColorAt(1, QColor(Qt::yellow).darker());
        painter.setBrush(gradient);
        painter.setPen(Qt::NoPen);
        painter.drawPath(textPath);

        // Wavy line under the text
        const int textWidth = largeFontMetrics.boundingRect(message).width();
        QPainterPath underLine;
        underLine.moveTo(50, 100);
        underLine.quadTo(textWidth/4.0 + 50, 120, textWidth/2.0 + 50, 100);
        underLine.quadTo(3 * textWidth/4.0 + 50, 80, textWidth + 50, 100);
        painter.drawPath(underLine);
    }

private:
    const QString message = "The Global Village";
    QFont largeFont;
    QFontMetrics largeFontMetrics = QFontMetrics(QFont());
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Hourses hourses;
    hourses.setFixedSize(700, 350);
    hourses.show();

    return app.exec();
}

#include "main.moc"
