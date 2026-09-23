#ifndef FIBEVENT_H
#define FIBEVENT_H

#include <QEvent>

static const QEvent::Type FibEventType = static_cast<QEvent::Type>(QEvent::User + 4242);

class FibEvent : public QEvent
{
public:
    explicit FibEvent(int result)
        : QEvent(FibEventType), value(result)
    {
    }

    int result() const
    {
        return value;
    }

private:
    int value;
};
#endif
