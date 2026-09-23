#include <QApplication>
#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include <QTranslator>
#include <QLayout>
#include <QStyleFactory>
#include <QFile>
#include <QDebug>

class ImageButton : public QPushButton
{
    Q_OBJECT

public:
    explicit ImageButton(const QString& name, const QString& imageFile, const QString& soundFile, QWidget* parent = nullptr)
        : QPushButton(parent) {
        this->soundFile = soundFile;
        this->pixmap = QPixmap(imageFile);
        this->name = name;
        setIcon(this->pixmap);
        connect(this, &ImageButton::clicked, this, &ImageButton::sayName);
        const int fontHeight = QFontMetrics(font()).height();
        setFixedSize(pixmap.size() + QSize(2 * offsetAroundContents, 2 * offsetAroundContents + fontHeight + spaceBetweenPictureAndText));
        setFlat(true);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        QPushButton::paintEvent(event);

        QPainter painter(this);
        painter.drawPixmap(offsetAroundContents, offsetAroundContents, pixmap);
        const int fontHeight = QFontMetrics(font()).height();
        painter.drawText(QRect(offsetAroundContents, offsetAroundContents + pixmap.height() + spaceBetweenPictureAndText, width() - (2 * offsetAroundContents), fontHeight), Qt::AlignCenter, name);
    }

protected slots:
    void sayName() const {
        QFile file(soundFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false) {
	    qDebug() << file.fileName() << "Cannot open.";
	}
        QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        stream.setCodec("Latin1");
#else
        stream.setEncoding(QStringConverter::Latin1);
#endif
        QString voiceText = stream.readLine();
        qDebug() << voiceText;
    }

private:
    const int offsetAroundContents = 3;
    const int spaceBetweenPictureAndText = 8;

    QString soundFile;
    QPixmap pixmap;
    QString name;
};

int main(int argc, char** argv)
{
    qputenv("QT_MAC_WANTS_LAYER", "1");

    QApplication app(argc, argv);

    app.setStyle(QStyleFactory::create("Fusion"));

    if (QApplication::arguments().count() > 1 && QApplication::arguments()[1] == "-dk") {
        qDebug() << "And now in danish.";
        QLocale::setDefault(QLocale(QLocale::Danish));

        const auto translator = new QTranslator;
        if (!translator->load(":/translations/resources_da.qm")) {
            qWarning() << "Cannot load the translator.";
        }
        app.installTranslator(translator);
    }

    const auto birdButton = new ImageButton(QObject::tr("Bird"), ":/images/bird.png", ":/sounds/bird.txt");
    const auto dogButton = new ImageButton(QObject::tr("Dog"), ":/images/dog.png", ":/sounds/dog.txt");

    QWidget top;
    top.resize(300, 200);

    const auto topLayout = new QHBoxLayout(&top);
    topLayout->addWidget(birdButton, 0, Qt::AlignHCenter);
    topLayout->addWidget(dogButton, 0, Qt::AlignHCenter);

    top.show();

    return app.exec();
}

#include "main.moc"
