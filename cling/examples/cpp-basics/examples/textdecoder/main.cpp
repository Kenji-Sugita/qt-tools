#include <QCoreApplication>
#include <QTextCodec>
#include <QTextDecoder>
#include <QDebug>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // Qt takes ownership of QTextCocdde instances.
    QTextCodec* const codec = QTextCodec::codecForName("UTF-8");
    QTextDecoder* const decoder = codec->makeDecoder();

    // Mahjong wind tiles.
    const char* const utf8WindTiles = "\xf0\x9f\x80\x80"
                                      "\xf0\x9f\x80\x81"
                                      "\xf0\x9f\x80\x82"
                                      "\xf0\x9f\x80\x83";

    QString string;
    for (const char* cp = utf8WindTiles; *cp != '\0'; ++cp) {
	const QString nextString = decoder->toUnicode(cp, 1);
	if (nextString.isEmpty()) {
	    qDebug() << "Skip";
	} else {
	    string += nextString;
	    qDebug() << "Added" << string.toUtf8().constData();
	}
    }
    delete decoder;

    qDebug() << string.toUtf8().constData();
}
