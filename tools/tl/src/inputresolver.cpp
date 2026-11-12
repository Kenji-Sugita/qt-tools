// CLI で与えられた入力源を実際の翻訳本文へ変換する実装。

#include "inputresolver.h"

#include <QFile>
#include <QTextStream>

#ifdef Q_OS_UNIX
#include <poll.h>
#include <unistd.h>
#endif

namespace {

bool stdinHasData()
{
#ifdef Q_OS_UNIX
    if (::isatty(STDIN_FILENO)) {
        return false;
    }

    pollfd stdinPoll{};
    stdinPoll.fd = STDIN_FILENO;
    stdinPoll.events = POLLIN;
    const int pollResult = ::poll(&stdinPoll, 1, 0);
    if (pollResult <= 0) {
        return false;
    }

    return (stdinPoll.revents & POLLIN) != 0;
#else
    return true;
#endif
}

QString readAllStdin()
{
    QTextStream in(stdin);
    return in.readAll();
}

TlError makeError(TlErrorCode code, const QString &message)
{
    return {code, message};
}

} // namespace

Result<TranslationRequest> InputResolver::resolve(const CliOptions &options) const
{
    const bool stdinAvailable = stdinHasData();

    if (stdinAvailable && options.filePath.has_value()) {
        return Result<TranslationRequest>::failure(
            makeError(TlErrorCode::InvalidArguments,
                      QStringLiteral("cannot combine standard input with --file")));
    }

    TranslationRequest request;
    if (options.filePath.has_value()) {
        QFile file(*options.filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return Result<TranslationRequest>::failure(
                makeError(TlErrorCode::FileReadFailed,
                          QStringLiteral("cannot read file '%1'").arg(*options.filePath)));
        }
        request.text = QString::fromUtf8(file.readAll());
    } else if (options.text.has_value()) {
        request.text = *options.text;
    } else {
        request.text = readAllStdin();
    }

    return Result<TranslationRequest>::success(std::move(request));
}
