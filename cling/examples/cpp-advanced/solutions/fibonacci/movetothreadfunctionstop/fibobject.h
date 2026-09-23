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

public slots:
    void start();

private:
    int fibonacci(int n);

    struct {
        QThread* createdThread;
    } d;
};
#endif
