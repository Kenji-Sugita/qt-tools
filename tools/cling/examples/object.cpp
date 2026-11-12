#include <QObject>
#include <QDebug>

class Object : public QObject
{
    Q_OBJECT

public:
    Object() {
        qDebug() << "ctor";
        setObjectName("Object");
    }

signals:
    void hello();
};

void object()
{
    Object object{};
    QObject::connect(&object, &Object::hello, []() { qDebug() << "Hello!"; });
    qDebug() << object.objectName();
    emit object.hello();
}

#include "object.moc"
