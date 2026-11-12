#ifndef FIBTHREAD_H
#define FIBTHREAD_H

#include <QThread>

class FibThread : public QThread
{
    Q_OBJECT

public:
    explicit FibThread(QObject* receiver, QObject* parent = nullptr);
    void run() override;

private:
    const int maxFib = 1000;

    int fibonacci(int);

    struct {
        QObject* receiver;
    } d;
};
#endif
