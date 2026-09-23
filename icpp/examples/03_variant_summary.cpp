// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QVariantList の値を読みやすい QString に要約してください。
// main() は書かず、REPL で扱いやすい短いコードにしてください。

#include <QString>
#include <QVariant>

QString summarizeVariants(const QVariantList& values)
{
    QStringList parts;
    int index = 0;
    for (const QVariant& value : values) {
        parts << QString("%1:%2=%3")
                     .arg(index++)
                     .arg(value.typeName())
                     .arg(value.toString());
    }
    return parts.join(", ");
}

QVariantList sampleValues{123, true, QString("Qt"), 45.6};
auto variantSummary = summarizeVariants(sampleValues);
