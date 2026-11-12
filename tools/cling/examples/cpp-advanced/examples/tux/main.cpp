#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QBitmap>
#include <QTimer>

class ClickToQuitEventFilter : public QObject
{
    Q_OBJECT

public:
    ClickToQuitEventFilter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

protected:
    bool eventFilter(QObject*, QEvent* event)
    {
        if (event->type() == QEvent::MouseButtonPress) {
            qApp->quit();
        }
        return false;
    }
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QLabel topLevelLabel;
    topLevelLabel.installEventFilter(new ClickToQuitEventFilter(&topLevelLabel));
    topLevelLabel.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    QPixmap pixmap(":/images/tux.png");
    topLevelLabel.setPixmap(pixmap);
    topLevelLabel.setMask(pixmap.mask());
    topLevelLabel.show();

    return app.exec();
}

#include "main.moc"
