#include "fibthread.h"

FibThread::FibThread(QObject* parent)
    : QThread(parent)
{
}

void FibThread::run()
{
    for (int i = 1; i < maxFib; i++) {
        const int fib = fibonacci(i);
        if (isInterruptionRequested()) {
            return;
        }
        emit fibFound(fib);
    }
}

int FibThread::fibonacci(int n)
{
    if (isInterruptionRequested()) {
        return -1;
    }

    if (n == 0 || n == 1) {
        return 1;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}
