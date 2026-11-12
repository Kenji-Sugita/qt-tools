// 複数の翻訳バックエンドを差し替えるための共通インターフェース。
#pragma once

#include "result.h"

#include <QString>

class TranslationProvider
{
public:
    virtual ~TranslationProvider() = default;

    virtual Result<QString> detectLanguage(const QString &text) = 0;
    virtual Result<QString> translate(const QString &text,
                                      const QString &sourceLanguage,
                                      const QString &targetLanguage) = 0;
};
