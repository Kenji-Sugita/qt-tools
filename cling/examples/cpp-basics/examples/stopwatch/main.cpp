#include <QApplication>
#include <QLabel>
#include <QElapsedTimer>
#include <QTimer>
#include <QDebug>

QFont fixedFont()
{
    static QFont platformDependFont = []() {
        QFont aFont;
#if defined(Q_OS_MACOS)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        // The window is not displayed when this font is set.
        aFont = QFont("Osaka");
        aFont.setStyleName("Regular-Mono");
#endif
#elif defined(Q_OS_LINUX)
        aFont = QFont("DejaVuSansMono");
#elif defined(Q_OS_WIN)
        aFont = QFont("Lucida Console");
#endif
        return aFont;
    }();

    return platformDependFont;
}

class StopWatch : public QLabel
{
    Q_OBJECT

public:
    explicit StopWatch(QWidget* parent = nullptr)
        : QLabel(parent) {
        setFont(fixedFont());

        QFont largeFont = font();
        largeFont.setPointSize(72);
        setFont(largeFont);

        const auto timer = new QTimer(this);
        setAlignment(Qt::AlignCenter);
        setText("0:00");
        connect(timer, &QTimer::timeout, this, &StopWatch::shot);
        time.start();
        timer->start(100);
    }

protected slots:
    void shot() {
        const int seconds = time.elapsed() / 1000;
        setText(QString("%1:%2")
                .arg(seconds / 60)
                .arg(seconds % 60, 2, 10, QChar('0')));
    }

private:
    QElapsedTimer time;
};

int main(int argc, char** argv)
{
    QApplication app( argc, argv );

    StopWatch watch;
    watch.setMinimumSize(1.2 * watch.sizeHint());

    watch.show();

    return app.exec();
}
#include "main.moc"
