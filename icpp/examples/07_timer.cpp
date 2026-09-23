// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QTimer::singleShot を使い、少し遅れて qDebug メッセージを出してください。
// main() は書かず、REPL でそのまま使えるコードにしてください。

#include <QDebug>
#include <QTimer>

void later(int milliseconds = 1000)
{
    QTimer::singleShot(milliseconds, []() {
        qDebug() << "timer fired";
    });
}
