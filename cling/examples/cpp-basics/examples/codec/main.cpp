#include <QApplication>
#include <QMap>
#include <QTabWidget>
#include <QLabel>
#include <QTextCodec>
#include <QFile>
#include <QDebug>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Key: Language file suffix, Value: Encoding
    QMap<QString, QString> encodings {
        { "chinese", "Big5" },
        { "japanese", "eucJP" },
        { "danish", "ISO8859-1" },
        { "greek", "ISO8859-7" },
        { "english", "ISO8859-1" },
        { "japanese-wrong-encoding", "ISO8859-1" } };

    QTabWidget languageSamples;
    QStringList languageFileSuffixes = encodings.keys();
    languageFileSuffixes.sort();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    for (const QString& fileSuffix : std::as_const(languageFileSuffixes)) {
#else
    for (const QString& fileSuffix : qAsConst(languageFileSuffixes)) {
#endif
        QFile file(":/texts/text-" + fileSuffix);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false) {
	    qDebug() << file.fileName() << "Cannot open.";
	}
        const QByteArray rawContents = file.readAll();
        const QString codecName = encodings.value(fileSuffix);
        const QTextCodec* const codecOfContents = QTextCodec::codecForName(codecName.toUtf8());
        if (codecOfContents != 0) {
            const QString text = codecOfContents->toUnicode(rawContents);

            const auto descriptionLabel = new QLabel(text);
            descriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            descriptionLabel->setWordWrap(true);

            QString tabText = fileSuffix;
            tabText[0] = tabText.at(0).toUpper();

            languageSamples.addTab(descriptionLabel, tabText);
        } else {
            qDebug() << "Cannot find" << codecName;
        }
    }

    languageSamples.show();

    return app.exec();
}
