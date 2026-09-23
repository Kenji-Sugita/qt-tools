// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QStringList を作成して変換し、.p で確認しやすい変数を残してください。
// main() は書かず、REPL でそのまま使えるコードにしてください。

#include <QString>
#include <QStringList>

QStringList names{"Ada", "Grace", "Linus", "Bjarne"};

QStringList upperNames()
{
    QStringList result;
    for (const QString& name : names) {
        result << name.toUpper();
    }
    return result;
}

auto joinedNames = names.join(", ");
auto upperJoinedNames = upperNames().join(" | ");
