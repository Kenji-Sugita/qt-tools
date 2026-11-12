#ifndef SLOT_H
#define SLOT_H
#include <QObject>

class Receiver : public QObject
{
    Q_OBJECT

public:
    explicit Receiver(QObject* parent = nullptr);

public slots:
    void aSlot();
    void anOtherSlot(int number, QObject* widget);
};
#endif
