#include <QApplication>
#include <QPushButton>
#include <QThread>
#include <QDebug>

class Data
{
public:
    Data()
        : pointerToInt(nullptr), string(), intData(-1)
    {
    }

    explicit Data(int i, int* ip, const QString& s)
        : pointerToInt(ip), string(s), intData(i)
    {
    }

    QString description() const
    {
        if (pointerToInt) {
            return QString("%1, %2, %3").arg(intData).arg(*pointerToInt).arg(string);
        } else {
            return QString("%1, %2, %3").arg(intData).arg("null").arg(string);
        }
    }

private:
    int* pointerToInt;
    QString string;
    int intData;
};

Q_DECLARE_METATYPE(Data);

class Sender : public QPushButton
{
    Q_OBJECT

public:
    Sender(QWidget* parent = nullptr)
        : QPushButton("Send a data to the thread", parent)
    {
        connect(this, &Sender::clicked, this, &Sender::sendData);
        intData = 42;
    }

signals:
    void data(const Data&);

protected slots:
    void sendData() {
        emit data(Data());
        emit data(Data(10, &intData, "Hi!"));
    }

private:
    int intData;
};


class Receiver : public QObject
{
    Q_OBJECT

public:
    Receiver(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

public slots:
    void data(const Data& data)
    {
        qDebug().noquote() << "Receiver says:" << data.description();
    }
};

class Thread : public QThread
{
    Q_OBJECT

public:
    Thread(Sender* emitter)
        : emitter(emitter), receiver(nullptr)
    {
    }

    ~Thread()
    {
        if (isRunning()) {
            quit();
            wait();
            delete receiver;
            receiver = nullptr;
        }
    }

    void run()
    {
        receiver = new Receiver;
        connect(emitter, &Sender::data, receiver, &Receiver::data);
        exec();
    }

private:
    Sender* const emitter;
    Receiver* receiver;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qRegisterMetaType<Data>();
#endif

    Sender sender;

    Thread thread(&sender);
    thread.start();

    sender.show();

    return app.exec();
}

#include "main.moc"
