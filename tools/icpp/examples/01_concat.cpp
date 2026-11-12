// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// 任意個数の値を受け取り、文字列化して結合する concat 関数を定義してください。
// main() は書かず、REPL でそのまま使えるコードにしてください。

#include <QString>
#include <QVariant>
#include <utility>

inline QString concatValueToString(const char* value)
{
    return QString::fromUtf8(value);
}

template <size_t N>
QString concatValueToString(const char (&value)[N])
{
    return QString::fromUtf8(value);
}

template <typename T>
QString concatValueToString(T&& value)
{
    return QVariant::fromValue(std::forward<T>(value)).toString();
}

template <typename... Args>
QString concat(Args&&... args)
{
    QString result;
    ((result += concatValueToString(std::forward<Args>(args))), ...);
    return result;
}

auto concatExample = concat(12, 34, "xyz");
