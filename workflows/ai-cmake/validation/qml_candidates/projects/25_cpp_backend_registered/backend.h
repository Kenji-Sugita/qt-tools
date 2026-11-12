#pragma once

#include <QObject>
#include <QString>

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString message READ message CONSTANT)

public:
    explicit Backend(QObject *parent = nullptr);
    QString message() const;
};
