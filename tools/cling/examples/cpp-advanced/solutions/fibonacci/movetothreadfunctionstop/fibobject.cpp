#include "fibobject.h"
#include <QThread>
#include <QDebug>

const int MAXFIB = 1000;

FibObject::FibObject(QObject* parent) :
    QObject(parent)
{
    d.createdThread = QThread::currentThread();
    qDebug() << "created:" << d.createdThread << "current:" << QThread::currentThread() << "affinity:" << thread();
}

void FibObject::start()
{
    qDebug() << "created:" << d.createdThread << "current:" << QThread::currentThread() << "affinity:" << thread();
    for (int i = 1; i < MAXFIB; i++) {
        int fib = fibonacci(i);
        if (thread()->isInterruptionRequested()) {
	    qDebug() << "created:" << d.createdThread << "current:" << QThread::currentThread() << "affinity:" << thread();
	    Q_ASSERT(QThread::currentThread() == thread());
	    if (thread() != d.createdThread) {
		moveToThread(d.createdThread);
            }
	    qDebug() << "created:" << d.createdThread << "current:" << QThread::currentThread() << "affinity:" << thread();
            return;
	}
        emit fibFound(fib);
    }
}

int FibObject::fibonacci(int n)
{
    if (thread()->isInterruptionRequested()) {
        return -1;
    }

    if (n == 0 || n == 1) {
        return 1;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}
