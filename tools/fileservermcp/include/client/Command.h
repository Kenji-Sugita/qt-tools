#pragma once

#include <QString>
#include <QStringList>
#include <functional>

struct Command
{
    QString usage;
    QString description;
    std::function<int(const QStringList &)> run;
};
