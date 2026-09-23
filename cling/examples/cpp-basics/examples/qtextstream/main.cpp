#include <QCoreApplication>
#include <QTextStream>
#include <QFile>
#include <QDebug>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    QFile file("file.txt");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        QString string;
        stream >> string;
        int number;
        stream >> number;
        qDebug() <<  string << "--" << number;
        file.close();
        file.remove();
    } else if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "HelloWorld ";
        stream << 4711;
        file.close();
        qDebug() << "File written.";
    }
}
