#include "BoardModel.h"
#include "ImageImporter.h"

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>

class ImageImporterTest : public QObject {
    Q_OBJECT

private slots:
    void importsRasterImageAsEmbeddedPng();
    void importsSvgAsEmbeddedPng();
    void importsClipboardImageAtRequestedCenter();
    void boardModelSavesAndLoadsImageElements();
};

void ImageImporterTest::importsRasterImageAsEmbeddedPng()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QStringList suffixes{
        QStringLiteral("png"),
        QStringLiteral("jpg"),
        QStringLiteral("bmp"),
    };

    for (const QString &suffix : suffixes) {
        const QString filePath = directory.filePath(QStringLiteral("sample.%1").arg(suffix));
        QImage source(32, 24, QImage::Format_RGB32);
        source.fill(QColor(QStringLiteral("#ef4444")));
        QVERIFY2(source.save(filePath), qPrintable(filePath));

        DrawingElement element;
        QString error;
        QVERIFY2(ImageImporter::importFromFile(filePath, QSize(1920, 1080), &element, &error), qPrintable(error));
        QCOMPARE(element.type(), ElementType::Image);
        QCOMPARE(element.imageMimeType(), QStringLiteral("image/png"));
        QVERIFY(!element.imageData().isEmpty());
        QCOMPARE(element.rect().size(), QSizeF(32, 24));

        QImage decoded;
        QVERIFY(decoded.loadFromData(element.imageData(), "PNG"));
        QCOMPARE(decoded.size(), QSize(32, 24));
    }
}

void ImageImporterTest::importsSvgAsEmbeddedPng()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString filePath = directory.filePath(QStringLiteral("sample.svg"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"40\" height=\"30\">"
               "<rect width=\"40\" height=\"30\" fill=\"#22c55e\"/></svg>");
    file.close();

    DrawingElement element;
    QString error;
    QVERIFY2(ImageImporter::importFromFile(filePath, QSize(1920, 1080), &element, &error), qPrintable(error));
    QCOMPARE(element.type(), ElementType::Image);
    QCOMPARE(element.imageMimeType(), QStringLiteral("image/png"));
    QCOMPARE(element.rect().size(), QSizeF(40, 30));

    QImage decoded;
    QVERIFY(decoded.loadFromData(element.imageData(), "PNG"));
    QCOMPARE(decoded.size(), QSize(40, 30));
}

void ImageImporterTest::importsClipboardImageAtRequestedCenter()
{
    QImage source(80, 40, QImage::Format_ARGB32_Premultiplied);
    source.fill(QColor(QStringLiteral("#f97316")));

    DrawingElement element;
    QString error;
    QVERIFY2(ImageImporter::importFromImage(source, QSize(1920, 1080), QPointF(300, 200), &element, &error), qPrintable(error));
    QCOMPARE(element.type(), ElementType::Image);
    QCOMPARE(element.imageMimeType(), QStringLiteral("image/png"));
    QCOMPARE(element.rect(), QRectF(260, 180, 80, 40));

    QImage decoded;
    QVERIFY(decoded.loadFromData(element.imageData(), "PNG"));
    QCOMPARE(decoded.size(), QSize(80, 40));
}

void ImageImporterTest::boardModelSavesAndLoadsImageElements()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QImage source(8, 6, QImage::Format_ARGB32_Premultiplied);
    source.fill(QColor(QStringLiteral("#3b82f6")));
    QByteArray data;
    QBuffer buffer(&data);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(source.save(&buffer, "PNG"));

    BoardModel model;
    model.addImage(QRectF(10, 20, 80, 60), data, QStringLiteral("image/png"));
    QCOMPARE(model.currentPage().elements().size(), 1);
    const DrawingElement saved = model.currentPage().elements().first();
    QCOMPARE(saved.type(), ElementType::Image);
    QCOMPARE(saved.rect(), QRectF(10, 20, 80, 60));
    QCOMPARE(saved.imageData(), data);

    const QString filePath = directory.filePath(QStringLiteral("whiteboard.json"));
    QVERIFY(model.saveToFile(filePath));
    BoardModel loaded;
    QVERIFY(loaded.loadFromFile(filePath));
    QCOMPARE(loaded.currentPage().elements().size(), 1);
    const DrawingElement loadedElement = loaded.currentPage().elements().first();
    QCOMPARE(loadedElement.type(), ElementType::Image);
    QCOMPARE(loadedElement.rect(), QRectF(10, 20, 80, 60));
    QCOMPARE(loadedElement.imageMimeType(), QStringLiteral("image/png"));
    QCOMPARE(loadedElement.imageData(), data);
}

QTEST_MAIN(ImageImporterTest)
#include "tst_imageimporter.moc"
