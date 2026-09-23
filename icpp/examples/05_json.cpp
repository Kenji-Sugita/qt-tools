// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QVariantMap を QJsonDocument で見やすい JSON 文字列に変換してください。
// main() は書かず、REPL でそのまま使えるコードにしてください。

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVariant>

QString toPrettyJson(const QVariantMap& values)
{
    const QJsonDocument document = QJsonDocument::fromVariant(values);
    return QString::fromUtf8(document.toJson(QJsonDocument::Indented));
}

QVariantMap settings{
    {"theme", "dark"},
    {"fontSize", 14},
    {"lineNumbers", true},
};

auto settingsJson = toPrettyJson(settings);
