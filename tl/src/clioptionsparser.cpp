// tl の CLI 構文だけを扱うパーサ実装。

#include "clioptionsparser.h"

#include <QCommandLineParser>

namespace {

TlError invalidArguments(const QString &message)
{
    return {TlErrorCode::InvalidArguments, message};
}

} // namespace

Result<CliOptions> CliOptionsParser::parse(QCoreApplication &app) const
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Translate between Japanese and English."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("text"), QStringLiteral("text to translate"));
    parser.addOption({QStringList{QStringLiteral("file")},
                      QStringLiteral("Read input text from file"),
                      QStringLiteral("path")});
    parser.process(app);

    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() > 1) {
        return Result<CliOptions>::failure(invalidArguments(QStringLiteral("too many positional arguments")));
    }

    CliOptions options;
    if (!positionalArguments.isEmpty()) {
        options.text = positionalArguments.first();
    }
    if (parser.isSet(QStringLiteral("file"))) {
        options.filePath = parser.value(QStringLiteral("file"));
    }

    if (options.text.has_value() && options.filePath.has_value()) {
        return Result<CliOptions>::failure(
            invalidArguments(QStringLiteral("cannot combine text input with --file")));
    }

    return Result<CliOptions>::success(std::move(options));
}
