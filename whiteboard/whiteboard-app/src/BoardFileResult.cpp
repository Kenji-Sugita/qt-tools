#include "BoardFileResult.h"

bool BoardFileResult::ok() const
{
    return status == Status::Success;
}

bool BoardFileResult::missing() const
{
    return status == Status::NotFound;
}

QJsonObject BoardFileResult::errorJson() const
{
    if (ok())
        return {};
    QJsonObject object{{QStringLiteral("stage"), stage}, {QStringLiteral("message"), message}};
    if (expectedBytes >= 0) {
        object.insert(QStringLiteral("expectedBytes"), expectedBytes);
        object.insert(QStringLiteral("writtenBytes"), writtenBytes);
    }
    if (errorCode != 0)
        object.insert(QStringLiteral("code"), errorCode);
    return object;
}
