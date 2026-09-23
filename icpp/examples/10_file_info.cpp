// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QFileInfo でパス情報を調べ、短い QString の要約を返してください。
// main() は書かず、REPL でそのまま使えるコードにしてください。

#include <QDir>
#include <QFileInfo>
#include <QString>

QString describePath(const QString& path)
{
    const QFileInfo info(path);
    return QString("%1 | exists=%2 | dir=%3 | size=%4")
        .arg(QDir::toNativeSeparators(info.absoluteFilePath()))
        .arg(info.exists() ? "true" : "false")
        .arg(info.isDir() ? "true" : "false")
        .arg(info.size());
}

auto currentDirSummary = describePath(".");
