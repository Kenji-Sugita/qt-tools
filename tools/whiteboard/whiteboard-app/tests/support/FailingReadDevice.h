#pragma once

#include <QIODevice>

#include <cstring>

class FailingReadDevice : public QIODevice {
public:
    explicit FailingReadDevice(QByteArray prefix) : m_prefix(std::move(prefix))
    {
        open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    }
    bool isSequential() const override { return true; }

protected:
    qint64 readData(char *data, qint64 size) override
    {
        if (m_offset == m_prefix.size()) {
            setErrorString(QStringLiteral("Injected read failure after a valid JSON prefix"));
            return -1;
        }
        const qint64 count = qMin(size, qint64(m_prefix.size() - m_offset));
        std::memcpy(data, m_prefix.constData() + m_offset, size_t(count));
        m_offset += count;
        return count;
    }
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    QByteArray m_prefix;
    qsizetype m_offset = 0;
};
