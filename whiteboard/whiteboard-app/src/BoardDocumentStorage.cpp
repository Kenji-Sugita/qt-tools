#include "BoardDocumentStorage.h"

#include "BoardDocumentValidator.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>

#include <filesystem>
#include <system_error>

namespace {
BoardFileResult failure(const QString &stage, const QString &message, int errorCode = 0)
{
    return {BoardFileResult::Status::Failure, stage, message, -1, -1, errorCode};
}

std::filesystem::path nativePath(const QString &path)
{
#ifdef Q_OS_WIN
    return std::filesystem::path(path.toStdWString());
#else
    const QByteArray bytes = QFile::encodeName(path);
    return std::filesystem::path(bytes.constData());
#endif
}

BoardFileResult inspectReadPath(const QString &filePath)
{
    const auto path = nativePath(QFileInfo(filePath).absoluteFilePath());
    std::error_code error;
    auto status = std::filesystem::symlink_status(path, error);
    if (error && error != std::errc::no_such_file_or_directory)
        return failure(QStringLiteral("open"), QString::fromStdString(error.message()));
    if (!error && std::filesystem::exists(status)) {
        status = std::filesystem::status(path, error);
        if (error || !std::filesystem::is_regular_file(status))
            return failure(QStringLiteral("open"), error ? QString::fromStdString(error.message()) : QStringLiteral("The save path is not a regular file."));
        return {BoardFileResult::Status::Success, {}, {}};
    }
    // A missing descendant of a dangling symlink is not a new document.
    auto parent = path.parent_path();
    while (!parent.empty()) {
        error.clear();
        status = std::filesystem::symlink_status(parent, error);
        if (error && error != std::errc::no_such_file_or_directory)
            return failure(QStringLiteral("open"), QString::fromStdString(error.message()));
        if (!error && std::filesystem::exists(status)) {
            status = std::filesystem::status(parent, error);
            if (error || !std::filesystem::is_directory(status))
                return failure(QStringLiteral("open"), error ? QString::fromStdString(error.message()) : QStringLiteral("The save directory is not accessible."));
            return {BoardFileResult::Status::NotFound, QStringLiteral("open"), QStringLiteral("No saved document exists.")};
        }
        const auto next = parent.parent_path();
        if (next == parent)
            break;
        parent = next;
    }
    return failure(QStringLiteral("open"), QStringLiteral("The save path could not be inspected."));
}

QString resolvedName(const QString &name)
{
    QString current = QFileInfo(name).absoluteFilePath();
    for (int i = 0; i < 32; ++i) {
        const QFileInfo info(current);
        if (!info.isSymLink())
            break;
        current = info.symLinkTarget();
    }
    std::error_code error;
    const auto canonical = std::filesystem::weakly_canonical(nativePath(current), error);
    if (error)
        return QDir::cleanPath(current);
#ifdef Q_OS_WIN
    return QString::fromStdWString(canonical.native());
#else
    return QFile::decodeName(canonical.native().c_str());
#endif
}
}

BoardFileResult BoardDocumentStorage::read(const QString &filePath, BoardDocument *document)
{
    if (!document)
        return failure(QStringLiteral("read"), QStringLiteral("Missing document destination."));
    const auto inspected = inspectReadPath(filePath);
    if (!inspected.ok())
        return inspected;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return failure(QStringLiteral("open"), file.errorString(), file.error());
    return readDevice(file, document);
}

BoardFileResult BoardDocumentStorage::readDevice(QIODevice &device, BoardDocument *document)
{
    if (!document)
        return failure(QStringLiteral("read"), QStringLiteral("Missing document destination."));
    QByteArray bytes;
    char chunk[16384];
    while (true) {
        const qint64 count = device.read(chunk, sizeof(chunk));
        if (count < 0) {
            const auto *file = qobject_cast<QFileDevice *>(&device);
            return failure(QStringLiteral("read"), device.errorString(), file ? file->error() : 0);
        }
        if (count == 0)
            break;
        bytes.append(chunk, count);
    }
    if (const auto *file = qobject_cast<QFileDevice *>(&device)) {
        if (file->error() != QFileDevice::NoError)
            return failure(QStringLiteral("read"), file->errorString(), file->error());
    }
    QJsonParseError parseError;
    const auto json = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return failure(QStringLiteral("parse"), QStringLiteral("%1 (byte %2)").arg(parseError.errorString()).arg(parseError.offset));
    if (!json.isObject())
        return failure(QStringLiteral("validate"), QStringLiteral("The saved document must be a JSON object."));
    QString error;
    if (!BoardDocumentValidator::validate(json.object(), &error))
        return failure(QStringLiteral("validate"), error);
    const BoardDocument candidate = BoardDocument::fromJson(json.object());
    if (!BoardDocumentValidator::validateConnections(candidate, &error))
        return failure(QStringLiteral("validate"), error);
    *document = candidate;
    return {BoardFileResult::Status::Success, {}, {}};
}

BoardFileResult BoardDocumentStorage::write(const QString &filePath, const BoardDocument &document)
{
    const QString parent = QFileInfo(filePath).absolutePath();
    if (!QDir().mkpath(parent))
        return failure(QStringLiteral("create_directory"), QStringLiteral("Could not create the save directory: %1").arg(parent));
    QSaveFile file(filePath);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly))
        return failure(QStringLiteral("open"), file.errorString(), file.error());
    return writeDevice(file, document, [&file]() { return file.commit(); });
}

BoardFileResult BoardDocumentStorage::writeDevice(QIODevice &device, const BoardDocument &document,
                                                 const std::function<bool()> &commit)
{
    const QByteArray bytes = QJsonDocument(document.toJson()).toJson(QJsonDocument::Indented);
    const qint64 written = device.write(bytes);
    if (written != bytes.size()) {
        auto result = failure(QStringLiteral("write"), device.errorString());
        if (const auto *file = qobject_cast<QFileDevice *>(&device))
            result.errorCode = file->error();
        result.expectedBytes = bytes.size();
        result.writtenBytes = written;
        return result;
    }
    if (!commit || !commit()) {
        auto result = failure(QStringLiteral("commit"), device.errorString());
        if (const auto *file = qobject_cast<QFileDevice *>(&device))
            result.errorCode = file->error();
        result.expectedBytes = bytes.size();
        result.writtenBytes = written;
        return result;
    }
    return {BoardFileResult::Status::Success, {}, {}, bytes.size(), written};
}

bool BoardDocumentStorage::sameFile(const QString &left, const QString &right)
{
    if (left.isEmpty() || right.isEmpty())
        return false;
    std::error_code error;
    if (std::filesystem::equivalent(nativePath(left), nativePath(right), error) && !error)
        return true;
#ifdef Q_OS_WIN
    return resolvedName(left).compare(resolvedName(right), Qt::CaseInsensitive) == 0;
#else
    return resolvedName(left) == resolvedName(right);
#endif
}
