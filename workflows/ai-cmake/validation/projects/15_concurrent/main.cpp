#include <QCoreApplication>
#include <QtConcurrent>
#include "worker.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Worker worker;
    auto future = QtConcurrent::run([&worker] { return worker.calculate(); });
    future.waitForFinished();
    return future.result() == 1 ? 0 : 1;
}
