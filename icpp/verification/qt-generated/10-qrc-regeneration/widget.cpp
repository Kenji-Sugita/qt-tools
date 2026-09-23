#include <QFile>
#include <QString>
#include <QTextStream>

#include "qrc_resources.cpp"

QString resourceText() {
    QFile file(":/data/message.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "open failed";
    }
    QTextStream stream(&file);
    return stream.readAll().trimmed();
}
