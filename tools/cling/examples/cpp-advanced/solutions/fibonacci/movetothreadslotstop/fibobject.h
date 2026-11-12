#ifndef FIBOBJECT_H
#define FIBOBJECT_H

#include <QObject>
class QThread;

class FibObject : public QObject
{
    Q_OBJECT

public:
    explicit FibObject(QObject* parent = nullptr);

signals:
    void fibFound(int found);
    void stopped();

public slots:
    void start();
    void stop();

private:
    int fibonacci(int n);

    struct {
        volatile bool stop;
        QThread* createdThread;
    } d;
};
#endif
