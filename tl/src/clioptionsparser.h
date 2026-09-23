// QCommandLineParser を閉じ込めて CLI オプションを解釈する。
#pragma once

#include "clioptions.h"
#include "result.h"

#include <QCoreApplication>

class CliOptionsParser
{
public:
    Result<CliOptions> parse(QCoreApplication &app) const;
};
