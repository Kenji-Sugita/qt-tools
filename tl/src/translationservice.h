// 自動判定して翻訳方向を決める業務フロー層。
#pragma once

#include "inputresolver.h"
#include "result.h"
#include "translationprovider.h"

#include <QString>

struct TranslationResult {
    QString translatedText;
    QString sourceLanguage;
    QString targetLanguage;
};

class TranslationService
{
public:
    explicit TranslationService(TranslationProvider &provider);

    Result<TranslationResult> translateAuto(const TranslationRequest &request) const;

private:
    TranslationProvider &m_provider;
};
