// text / --file / stdin の入力源を確定して本文を読み込む。
#pragma once

#include "clioptions.h"
#include "result.h"

#include <QString>

struct TranslationRequest {
    QString text;
};

class InputResolver
{
public:
    Result<TranslationRequest> resolve(const CliOptions &options) const;
};
