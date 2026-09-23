#pragma once

#include <QMetaType>
#include <QSize>
#include <QDebug>

struct Data {
    Data() noexcept;
    Data& operator=(const Data& other) noexcept;
    Data& operator=(Data&& other) noexcept;
    Data(const Data& other) noexcept;
    Data(Data&& other) noexcept;
    virtual ~Data();

    QString type() const noexcept;
    QSize size() const noexcept;

    void setType(const QString& type) noexcept;
    void setSize(const QSize& size) noexcept;

//    operator QVariant() const; // Step 2
    bool operator==(const Data& other) const noexcept;

private:
    struct {
        QString type;
        QSize size;
    } d;
};

//Q_DECLARE_METATYPE(Data) // Step 1

QDebug operator<<(QDebug debug, const Data& data);
