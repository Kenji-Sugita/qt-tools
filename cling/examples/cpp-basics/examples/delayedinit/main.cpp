#include <QApplication>
#include <QPushButton>
#include <QTextEdit>
#include <QLayout>

class Sender : public QPushButton
{
    Q_OBJECT

public:
    explicit Sender(QWidget* parent = nullptr)
        : QPushButton("Press me", parent), counter(0) {
        connect(this, &Sender::clicked, this, &Sender::emitData);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        QMetaObject::invokeMethod(this, &Sender::emitData, Qt::QueuedConnection);
#else
        QMetaObject::invokeMethod(this, "emitData", Qt::QueuedConnection);
#endif
    }

signals:
    void data(int);

protected slots:
    void emitData() {
        emit data(counter++);
    }

private:
    int counter;
};

class Receiver : public QTextEdit
{
    Q_OBJECT

public:
    explicit Receiver(QWidget* parent = nullptr) : QTextEdit(parent) {}

public slots:
    void data(int i) {
        append(QString::number(i));
    }
};

int main(int argc, char** argv)
{
    QApplication app(argc,argv);

    const auto sender = new Sender;
    const auto receiver = new Receiver;

    QWidget top;
    const auto topLayout = new QVBoxLayout(&top);
    topLayout->addWidget(sender, 0, Qt::AlignHCenter);
    topLayout->addWidget(receiver);

    QObject::connect(sender, &Sender::data, receiver, &Receiver::data);

    top.show();

    return app.exec();
}

#include "main.moc"
