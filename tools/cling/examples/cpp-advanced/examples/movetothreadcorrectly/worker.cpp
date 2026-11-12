#include <QElapsedTimer>
#include <QEventLoop>
#include <QThread>
#include "worker.h"

Worker::Worker(QObject* parent) :
    QObject(parent)
{
    m_stop = false;
    m_parent_thread = thread();
}

void Worker::stop()
{
    m_stop = true;
}

void Worker::generate()
{
    m_stop = false;

    QElapsedTimer allTimer;
    allTimer.start();

    QElapsedTimer timer;
    timer.start();

    QEventLoop eventLoop;

    int i = 0;
    while (!m_stop) {
        ++i;

        if (timer.elapsed() >= 100) {
            emit dump(i);
            timer.restart();
            // stop() スロットが呼び出されるようにするために必要。
            eventLoop.processEvents(QEventLoop::AllEvents, 50);
        }
    }

    emit finished(tr("%1/sec").arg(i / (allTimer.elapsed() / 100.0)));
}
