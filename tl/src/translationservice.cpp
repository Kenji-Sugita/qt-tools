// 翻訳プロバイダを使って tl の翻訳ルールを実装する。

#include "translationservice.h"

namespace {

TlError makeError(TlErrorCode code, const QString &message)
{
    return {code, message};
}

QString trimTrailingNewline(QString text)
{
    while (text.endsWith(u'\n') || text.endsWith(u'\r')) {
        text.chop(1);
    }
    return text;
}

} // namespace

TranslationService::TranslationService(TranslationProvider &provider)
    : m_provider(provider)
{
}

Result<TranslationResult> TranslationService::translateAuto(const TranslationRequest &request) const
{
    const QString trimmed = trimTrailingNewline(request.text);
    if (trimmed.trimmed().isEmpty()) {
        return Result<TranslationResult>::failure(
            makeError(TlErrorCode::EmptyInput, QStringLiteral("empty input")));
    }

    auto detectResult = m_provider.detectLanguage(trimmed);
    if (!detectResult.ok()) {
        return Result<TranslationResult>::failure(detectResult.error());
    }

    const QString sourceLanguage = detectResult.value();
    QString targetLanguage;
    if (sourceLanguage == QStringLiteral("ja")) {
        targetLanguage = QStringLiteral("en");
    } else if (sourceLanguage == QStringLiteral("en")) {
        targetLanguage = QStringLiteral("ja");
    } else {
        return Result<TranslationResult>::failure(
            makeError(TlErrorCode::UnsupportedLanguage,
                      QStringLiteral("detected language '%1' is not supported").arg(sourceLanguage)));
    }

    auto translateResult = m_provider.translate(trimmed, sourceLanguage, targetLanguage);
    if (!translateResult.ok()) {
        return Result<TranslationResult>::failure(translateResult.error());
    }

    return Result<TranslationResult>::success(
        TranslationResult{translateResult.value(), sourceLanguage, targetLanguage});
}
