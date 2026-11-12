// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QVariantMap を作成し、確認しやすい key=value 形式の QString に整形してください。
// main() は書かず、REPL でそのまま使えるコードにしてください。

#include <QMap>
#include <QString>
#include <QVariant>

QString formatVariantMap(const QVariantMap& values)
{
    QStringList lines;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        lines << QString("%1=%2").arg(it.key(), it.value().toString());
    }
    return lines.join("\n");
}

QVariantMap user{
    {"name", "Ada"},
    {"role", "engineer"},
    {"score", 98},
};

auto userText = formatVariantMap(user);
