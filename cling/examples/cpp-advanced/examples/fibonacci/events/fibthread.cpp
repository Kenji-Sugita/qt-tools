#include "fibthread.h"
#include "fibevent.h"
#include <QCoreApplication>
#include <QThread>

FibThread::FibThread(QObject* receiver, QObject* parent)
    : QThread(parent)
{
    d.receiver = receiver;
}

void FibThread::run()
{
    for (int i = 1; i < maxFib; ++i) {
        const int fib = fibonacci(i);
        if (isInterruptionRequested()) {
            return;
        }
        QCoreApplication::postEvent(d.receiver, new FibEvent(fib));
    }
}

int FibThread::fibonacci(int i)
{
    if (isInterruptionRequested()) {
        return -1;
    }
    if (i == 0 || i == 1) {
        return 1;
    }
    return fibonacci(i - 1) + fibonacci(i - 2);
}
