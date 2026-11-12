// コマンドライン解析結果を保持する値型。
#pragma once

#include <QString>

#include <optional>

struct CliOptions {
    std::optional<QString> text;
    std::optional<QString> filePath;
};
