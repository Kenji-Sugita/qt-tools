#pragma once

#include <QJsonArray>
#include <QString>

class FileRepository
{
public:
    FileRepository(QString rootPath, qint64 maxSize);

    bool isValid(QString *error) const;
    QJsonArray listFiles(QString *error) const;
    QByteArray readFile(const QString &name, QString *mimeType, QString *error) const;
    qint64 maxSize() const;

private:
    bool isSafeFileName(const QString &name) const;
    QString absolutePathForName(const QString &name) const;

    QString m_rootPath;
    qint64 m_maxSize;
};
