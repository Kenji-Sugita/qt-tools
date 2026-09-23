#pragma once

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

class Counter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int value READ value CONSTANT)

public:
    explicit Counter(QObject *parent = nullptr);
    int value() const;
};
