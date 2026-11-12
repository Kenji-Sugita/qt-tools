#ifndef FIBTHREAD_H
#define FIBTHREAD_H

#include <QThread>

class FibThread : public QThread
{
    Q_OBJECT

public:
    explicit FibThread(QObject* parent = nullptr);
    void run() override;

signals:
    void fibFound(int found);

private:
    const int maxFib = 1000;

    int fibonacci(int n);
};
#endif
