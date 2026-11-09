#pragma once

#include <QIODevice>

class FailingWriteDevice : public QIODevice {
public:
    enum class Mode { Error, Short, Accept };
    explicit FailingWriteDevice(Mode mode) : m_mode(mode)
    {
        open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    }
    bool failCompletion(const QString &message)
    {
        setErrorString(message);
        return false;
    }

protected:
    qint64 readData(char *, qint64) override { return -1; }
    qint64 writeData(const char *, qint64 size) override
    {
        if (m_mode == Mode::Accept)
            return size;
        setErrorString(QStringLiteral("Injected write failure"));
        return m_mode == Mode::Error ? -1 : size - 1;
    }

private:
    Mode m_mode;
};
