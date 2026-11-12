#include <QCoreApplication>
#include "repository.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Repository repository;
    return repository.database().isValid() ? 0 : 0;
}
