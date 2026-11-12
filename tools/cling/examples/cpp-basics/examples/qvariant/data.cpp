#include "data.h"
// #include <QVariant> // Step 2

Data::Data() noexcept
{
    d.type = QString();
    d.size = QSize();
}

Data& Data::operator=(const Data& other) noexcept
{
    if (this != &other) {
        d.type = other.d.type;
        d.size = other.d.size;
    }

    return *this;
}

Data& Data::operator=(Data&& other) noexcept
{
    if (this != &other) {
        d.type = std::move(other.d.type);
        d.size = std::move(other.d.size);
    }

    return *this;
}

Data::Data(const Data& other) noexcept
{
    *this = other;
}

Data::Data(Data&& other) noexcept
{
    *this = std::move(other);
}

Data::~Data()
{
}

QString Data::type() const noexcept
{
    return d.type;
}

QSize Data::size() const noexcept
{
    return d.size;
}

void Data::setType(const QString& type) noexcept
{
    d.type = type;
}

void Data::setSize(const QSize& size) noexcept
{
    d.size = size;
}

//Data::operator QVariant() const              // Step 2
//{                                            // Step 2
//    return QVariant::fromValue<Data>(*this); // Step 2
//}                                            // Step 2

bool Data::operator==(const Data& other) const noexcept
{
    return d.type == other.d.type && d.size == other.d.size;
}

QDebug operator<<(QDebug debug, const Data& data)
{
    QDebugStateSaver saver(debug);

    debug.nospace() << "Data(" << data.type() << ", " << data.size() << ")";

    return debug;
}
