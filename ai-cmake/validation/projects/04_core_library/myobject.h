#pragma once

#include <QObject>
#include <QString>

class MyObject : public QObject
{
    Q_OBJECT

public:
    explicit MyObject(QObject *parent = nullptr);
    QString name() const;
};
