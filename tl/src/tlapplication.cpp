// CLI 解析から出力までをつなぐアプリケーション層の実装。

#include "tlapplication.h"

#include <QTextStream>

namespace {

constexpr int kExitSuccess = 0;
constexpr int kExitFailure = 1;

} // namespace

int TlApplication::run(QCoreApplication &app)
{
    auto optionsResult = m_cliOptionsParser.parse(app);
    if (!optionsResult.ok()) {
        printError(optionsResult.error().message);
        return kExitFailure;
    }

    auto requestResult = m_inputResolver.resolve(optionsResult.value());
    if (!requestResult.ok()) {
        printError(requestResult.error().message);
        return kExitFailure;
    }

    auto translationResult = m_translationService.translateAuto(requestResult.value());
    if (!translationResult.ok()) {
        printError(translationResult.error().message);
        return kExitFailure;
    }

    QTextStream out(stdout);
    out << translationResult.value().translatedText << Qt::endl;
    return kExitSuccess;
}

void TlApplication::printError(const QString &message) const
{
    QTextStream err(stderr);
    err << "tl: " << message << Qt::endl;
}
