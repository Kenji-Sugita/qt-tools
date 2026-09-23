#include <QApplication>
#include <QPushButton>
#include <QLayout>
#include <QPainter>
#include <QWindow>

class BlobWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BlobWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);

        const QRectF blobRect = rect();
        if (devicePixelRatio() > 1.0) {
            painter.setPen(QPen(Qt::black, 1.0/devicePixelRatio()));  // Thin pen
            painter.fillRect(blobRect, QColor(10, 50, 180, 60));
            painter.drawRect(blobRect.adjusted(0, 0, -0.5, -0.5));  // Thin border rectangle
        } else {
            painter.setPen(QPen(Qt::black, 1.0));
            painter.fillRect(blobRect, QColor(10, 50, 180, 60));
            painter.drawRect(blobRect.adjusted(0, 0, -1.0, -1.0));
        }
    }
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = nullptr)
        : QWidget(parent), blobWidget(new BlobWidget(this))
    {
        blobWidget->hide();

        const auto topLayout = new QGridLayout(this);
        for (int i = 0; i < 16; ++i)  {
            const auto button = new QPushButton(QString::number(i));
            topLayout->addWidget(button, i/4, i%4);
            topLayout->setAlignment(button, Qt::AlignCenter);
            connect(button, &QPushButton::clicked, this, &Widget::selectButton);
        }
    }

protected slots:
    void selectButton()
    {
        const auto selectedButton = static_cast<const QPushButton*>(sender());
        Q_ASSERT(selectedButton);
        blobWidget->raise();
        blobWidget->show();
        blobWidget->setGeometry(selectedButton->geometry().adjusted(-12, -12, 12, 12));
    }

private:
    BlobWidget* const blobWidget;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Widget widget;
    widget.show();

    return app.exec();
}

#include "main.moc"
