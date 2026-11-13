#include "FileRepository.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QMimeDatabase>

FileRepository::FileRepository(QString rootPath, qint64 maxSize)
    : m_rootPath(QDir(rootPath).canonicalPath())
    , m_maxSize(maxSize)
{
}

bool FileRepository::isValid(QString *error) const
{
    if (m_rootPath.isEmpty()) {
        if (error)
            *error = QStringLiteral("root directory does not exist");
        return false;
    }
    const QFileInfo info(m_rootPath);
    if (!info.isDir() || !info.isReadable()) {
        if (error)
            *error = QStringLiteral("root directory is not readable");
        return false;
    }
    return true;
}

QJsonArray FileRepository::listFiles(QString *error) const
{
    QString validError;
    if (!isValid(&validError)) {
        if (error)
            *error = validError;
        return {};
    }

    QJsonArray files;
    const QFileInfoList entries = QDir(m_rootPath).entryInfoList(QDir::Files | QDir::Readable,
                                                                 QDir::Name | QDir::IgnoreCase);
    for (const QFileInfo &entry : entries) {
        if (entry.isSymLink())
            continue;
        files.append(QJsonObject {
            {QStringLiteral("name"), entry.fileName()},
            {QStringLiteral("size"), entry.size()},
            {QStringLiteral("modified"), entry.lastModified().toString(Qt::ISODate)},
        });
    }
    return files;
}

QByteArray FileRepository::readFile(const QString &name, QString *mimeType, QString *error) const
{
    if (!isSafeFileName(name)) {
        if (error)
            *error = QStringLiteral("invalid file name");
        return {};
    }

    const QString path = absolutePathForName(name);
    const QFileInfo info(path);
    if (!info.exists() || !info.isFile() || info.isSymLink()) {
        if (error)
            *error = QStringLiteral("file not found");
        return {};
    }
    if (info.size() > m_maxSize) {
        if (error)
            *error = QStringLiteral("file is larger than max-size");
        return {};
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = file.errorString();
        return {};
    }
    if (mimeType)
        *mimeType = QMimeDatabase().mimeTypeForFile(info).name();
    return file.readAll();
}

qint64 FileRepository::maxSize() const
{
    return m_maxSize;
}

bool FileRepository::isSafeFileName(const QString &name) const
{
    return !name.isEmpty()
           && !name.contains(QStringLiteral(".."))
           && !name.contains(QLatin1Char('/'))
           && !name.contains(QLatin1Char('\\'));
}

QString FileRepository::absolutePathForName(const QString &name) const
{
    return QDir(m_rootPath).absoluteFilePath(name);
}
