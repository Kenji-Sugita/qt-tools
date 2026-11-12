#pragma once

#include <QSqlDatabase>

class Repository
{
public:
    QSqlDatabase database() const;
};
