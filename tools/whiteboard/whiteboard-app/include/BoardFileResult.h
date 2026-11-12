#pragma once

#include <QJsonObject>
#include <QString>

struct BoardFileResult {
    enum class Status { Success, NotFound, Failure };

    Status status = Status::Failure;
    QString stage;
    QString message;
    qint64 expectedBytes = -1;
    qint64 writtenBytes = -1;
    int errorCode = 0;

    bool ok() const;
    bool missing() const;
    QJsonObject errorJson() const;
};
