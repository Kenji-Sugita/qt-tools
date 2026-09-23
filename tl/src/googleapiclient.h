// Google Cloud Translation を扱う翻訳プロバイダ実装。
#pragma once

#include "result.h"
#include "translationprovider.h"

#include <QNetworkAccessManager>
#include <QString>

class GoogleTranslationProvider : public TranslationProvider
{
public:
    GoogleTranslationProvider();

    Result<QString> detectLanguage(const QString &text) override;
    Result<QString> translate(const QString &text,
                              const QString &sourceLanguage,
                              const QString &targetLanguage) override;

private:
    Result<QString> accessToken();

    QNetworkAccessManager m_networkAccessManager;
    QString m_accessToken;
};
