#ifndef OBJECT_H
#define OBJECT_H
#include <QDebug>

class Object
{
public:
    explicit Object(const QString& name) noexcept;
    Object(const Object& other) noexcept;
    virtual ~Object();

    QString name() const noexcept;
    void setName(const QString& name) noexcept;

private:
    struct {
        QString name;
    } d;
};

QDebug operator<<(QDebug debug, const Object& object);
#endif
