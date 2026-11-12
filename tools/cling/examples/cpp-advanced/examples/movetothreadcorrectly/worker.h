#pragma once

#include <QObject>

class QThread;

class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject* parent = nullptr);

signals:
    void dump(int i);
    void finished(const QString& message);

public slots:
    void generate();
    void stop();

private:
    volatile bool m_stop;
    QThread* m_parent_thread;
};
