// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QRegularExpression を使って QString から数値部分を抽出してください。
// main() は書かず、REPL で扱いやすい短いコードにしてください。

#include <QRegularExpression>
#include <QString>
#include <QStringList>

QStringList extractNumbers(const QString& text)
{
    QStringList result;
    const QRegularExpression pattern(QStringLiteral(R"(\d+)"));
    auto matches = pattern.globalMatch(text);
    while (matches.hasNext()) {
        result << matches.next().captured();
    }
    return result;
}

QString logLine = "items=12 errors=3 elapsed=456ms";
auto numbers = extractNumbers(logLine);
