#include "ImageExporter.h"

#include "BoardModel.h"

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>
#include <QXmlStreamReader>

class ImageExporterTest : public QObject {
    Q_OBJECT

private slots:
    void emptyPageHasNoExport();
    void selectedElementsOnlyAreExported();
    void pngAndSvgUseSameBoundsAndBackgroundChoice();
    void svgContainsVectorElementsAndReadableIds();
    void svgUsesPersistentElementIdsWhenAvailable();
    void svgSupportsEveryDrawingElementType();
    void svgEmbedsImageElements();
    void fileExtensionRulesAndSaving();
};

DrawingElement rectangle(const QRectF &rect, const QColor &color)
{
    DrawingElement element;
    element.setType(ElementType::Rectangle);
    element.setRect(rect);
    element.setColor(color);
    element.setStrokeWidth(4);
    return element;
}

void ImageExporterTest::emptyPageHasNoExport()
{
    const Page page;
    QVERIFY(!ImageExporter::hasExportableElements(page));
    QVERIFY(ImageExporter::exportBounds(page).isEmpty());
    QVERIFY(ImageExporter::renderPng(page, {}, ImageExporter::Background::Transparent).isNull());
    QVERIFY(ImageExporter::renderSvg(page, {}, ImageExporter::Background::Transparent).isEmpty());
}

void ImageExporterTest::selectedElementsOnlyAreExported()
{
    Page page;
    page.setElements({
        rectangle(QRectF(10, 20, 80, 60), QColor(QStringLiteral("#ff0000"))),
        rectangle(QRectF(300, 200, 100, 70), QColor(QStringLiteral("#0000ff"))),
    });

    QRect bounds;
    const QImage image = ImageExporter::renderPng(
        page, {1}, ImageExporter::Background::Transparent, &bounds);
    QVERIFY(!image.isNull());
    QVERIFY(bounds.left() > 250);
    int bluePixels = 0;
    int redPixels = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            if (pixel.blue() > pixel.red())
                ++bluePixels;
            if (pixel.red() > pixel.blue())
                ++redPixels;
        }
    }
    QVERIFY(bluePixels > 0);
    QCOMPARE(redPixels, 0);

    const QByteArray svg = ImageExporter::renderSvg(
        page, {1}, ImageExporter::Background::Transparent);
    QVERIFY(svg.contains("id=\"rectangle-2\""));
    QVERIFY(!svg.contains("id=\"rectangle-1\""));
    QVERIFY(!svg.contains("#ff0000"));
}

void ImageExporterTest::pngAndSvgUseSameBoundsAndBackgroundChoice()
{
    DrawingElement rotated = rectangle(QRectF(120, 90, 180, 70), QColor(QStringLiteral("#344054")));
    rotated.setRotationDegrees(32.0);
    rotated.setStrokeWidth(12);
    Page page;
    page.setElements({rotated});

    QRect pngBounds;
    QRect svgBounds;
    const QImage transparent = ImageExporter::renderPng(
        page, {}, ImageExporter::Background::Transparent, &pngBounds);
    const QByteArray svg = ImageExporter::renderSvg(
        page, {}, ImageExporter::Background::Transparent, &svgBounds);
    QCOMPARE(pngBounds, svgBounds);
    QVERIFY(pngBounds.contains(rotated.visualBounds().toAlignedRect()));
    QCOMPARE(transparent.pixelColor(0, 0).alpha(), 0);

    const QImage white = ImageExporter::renderPng(
        page, {}, ImageExporter::Background::White);
    QCOMPARE(white.pixelColor(0, 0), QColor(Qt::white));
    QVERIFY(!svg.contains("id=\"background\""));
    const QByteArray whiteSvg = ImageExporter::renderSvg(
        page, {}, ImageExporter::Background::White);
    QVERIFY(whiteSvg.contains("id=\"background\""));
}

void ImageExporterTest::svgContainsVectorElementsAndReadableIds()
{
    DrawingElement line;
    line.setType(ElementType::Line);
    line.setPoints({QPointF(20, 30), QPointF(180, 90)});
    line.setStrokeWidth(5);
    line.setStartArrowHead(ArrowHead::Open);
    line.setEndArrowHead(ArrowHead::Triangle);

    DrawingElement text;
    text.setType(ElementType::Text);
    text.setRect(QRectF(40, 120, 260, 80));
    text.setText(QStringLiteral("A < B & C"));

    Page page;
    page.setElements({line, text});
    const QByteArray svg = ImageExporter::renderSvg(
        page, {}, ImageExporter::Background::Transparent);

    QVERIFY(svg.contains("id=\"line-1\""));
    QVERIFY(svg.contains("id=\"text-2\""));
    QVERIFY(svg.contains("<line"));
    QVERIFY(svg.contains("<text"));
    QVERIFY(svg.contains("A &lt; B &amp; C"));
    QVERIFY(!svg.contains("<image"));

    QXmlStreamReader reader(svg);
    while (!reader.atEnd())
        reader.readNext();
    QVERIFY2(!reader.hasError(), qPrintable(reader.errorString()));
}

void ImageExporterTest::svgUsesPersistentElementIdsWhenAvailable()
{
    BoardModel model;
    model.addLine(QPointF(20, 30), QPointF(180, 90));
    model.addText(QPointF(40, 120), QStringLiteral("Persistent"));

    const QByteArray svg = ImageExporter::renderSvg(
        model.currentPage(), {}, ImageExporter::Background::Transparent);
    QVERIFY(svg.contains("id=\"element-1\""));
    QVERIFY(svg.contains("id=\"element-2\""));
    QVERIFY(!svg.contains("id=\"line-1\""));
    QVERIFY(!svg.contains("id=\"text-2\""));
}

void ImageExporterTest::svgSupportsEveryDrawingElementType()
{
    QVector<DrawingElement> elements;

    DrawingElement freehand;
    freehand.setType(ElementType::Freehand);
    freehand.setPoints({QPointF(10, 10), QPointF(30, 20), QPointF(50, 12)});
    elements.append(freehand);

    DrawingElement line;
    line.setType(ElementType::Line);
    line.setPoints({QPointF(70, 10), QPointF(120, 30)});
    elements.append(line);

    elements.append(rectangle(QRectF(140, 10, 50, 30), QColor(QStringLiteral("#112233"))));

    DrawingElement rounded = rectangle(QRectF(210, 10, 50, 30), QColor(QStringLiteral("#223344")));
    rounded.setType(ElementType::RoundedRectangle);
    rounded.setCornerRadius(8);
    elements.append(rounded);

    DrawingElement ellipse = rectangle(QRectF(280, 10, 50, 30), QColor(QStringLiteral("#334455")));
    ellipse.setType(ElementType::Ellipse);
    elements.append(ellipse);

    DrawingElement bezier;
    bezier.setType(ElementType::Bezier);
    bezier.setPoints({QPointF(10, 70), QPointF(40, 45), QPointF(70, 95), QPointF(100, 70)});
    elements.append(bezier);

    DrawingElement arc;
    arc.setType(ElementType::Arc);
    arc.setRect(QRectF(120, 50, 60, 60));
    arc.setArcStartAngle(20);
    arc.setArcSpanAngle(220);
    elements.append(arc);

    DrawingElement circle = rectangle(QRectF(200, 50, 60, 60), QColor(QStringLiteral("#445566")));
    circle.setType(ElementType::Circle);
    elements.append(circle);

    DrawingElement polyline;
    polyline.setType(ElementType::Polyline);
    polyline.setPoints({QPointF(280, 50), QPointF(330, 70), QPointF(290, 110)});
    polyline.setClosed(true);
    polyline.setFillColor(QColor(QStringLiteral("#80445566")));
    elements.append(polyline);

    DrawingElement text;
    text.setType(ElementType::Text);
    text.setRect(QRectF(10, 130, 240, 50));
    text.setText(QStringLiteral("all element types"));
    elements.append(text);

    QImage source(4, 3, QImage::Format_ARGB32_Premultiplied);
    source.fill(QColor(QStringLiteral("#f97316")));
    QByteArray data;
    QBuffer buffer(&data);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(source.save(&buffer, "PNG"));
    DrawingElement image;
    image.setType(ElementType::Image);
    image.setRect(QRectF(270, 130, 40, 30));
    image.setImageData(data);
    image.setImageMimeType(QStringLiteral("image/png"));
    elements.append(image);

    Page page;
    page.setElements(elements);
    const QByteArray svg = ImageExporter::renderSvg(page, {}, ImageExporter::Background::Transparent);
    const QStringList expectedIds = {
        QStringLiteral("freehand-1"),
        QStringLiteral("line-2"),
        QStringLiteral("rectangle-3"),
        QStringLiteral("rounded-rectangle-4"),
        QStringLiteral("ellipse-5"),
        QStringLiteral("bezier-6"),
        QStringLiteral("arc-7"),
        QStringLiteral("circle-8"),
        QStringLiteral("polyline-9"),
        QStringLiteral("text-10"),
        QStringLiteral("image-11"),
    };
    for (const QString &id : expectedIds)
        QVERIFY2(svg.contains(QStringLiteral("id=\"%1\"").arg(id).toUtf8()), qPrintable(id));

    QXmlStreamReader reader(svg);
    while (!reader.atEnd())
        reader.readNext();
    QVERIFY2(!reader.hasError(), qPrintable(reader.errorString()));
}

void ImageExporterTest::svgEmbedsImageElements()
{
    QImage source(4, 3, QImage::Format_ARGB32_Premultiplied);
    source.fill(QColor(QStringLiteral("#f97316")));
    QByteArray data;
    QBuffer buffer(&data);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(source.save(&buffer, "PNG"));

    DrawingElement image;
    image.setType(ElementType::Image);
    image.setRect(QRectF(10, 20, 40, 30));
    image.setImageData(data);
    image.setImageMimeType(QStringLiteral("image/png"));

    Page page;
    page.addElement(image);
    const QByteArray svg = ImageExporter::renderSvg(page, {}, ImageExporter::Background::Transparent);
    QVERIFY(svg.contains("<image"));
    QVERIFY(svg.contains("href=\"data:image/png;base64,"));
    QVERIFY(svg.contains("id=\"image-1\""));

    QXmlStreamReader reader(svg);
    while (!reader.atEnd())
        reader.readNext();
    QVERIFY2(!reader.hasError(), qPrintable(reader.errorString()));

    const QImage png = ImageExporter::renderPng(page, {}, ImageExporter::Background::Transparent);
    QVERIFY(!png.isNull());
}

void ImageExporterTest::fileExtensionRulesAndSaving()
{
    QCOMPARE(ImageExporter::withDefaultExtension(QStringLiteral("diagram"), ImageExporter::Format::Png),
             QStringLiteral("diagram.png"));
    QCOMPARE(ImageExporter::withDefaultExtension(QStringLiteral("diagram.svg"), ImageExporter::Format::Png),
             QStringLiteral("diagram.svg"));

    ImageExporter::Format format = ImageExporter::Format::Png;
    QVERIFY(ImageExporter::formatForFilePath(QStringLiteral("diagram.SVG"), ImageExporter::Format::Png, &format));
    QCOMPARE(format, ImageExporter::Format::Svg);
    QVERIFY(!ImageExporter::formatForFilePath(QStringLiteral("diagram.jpg"), ImageExporter::Format::Png, &format));

    Page page;
    page.setElements({rectangle(QRectF(10, 20, 80, 60), QColor(QStringLiteral("#123456")))});
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    ImageExporter::Options options;
    options.format = ImageExporter::Format::Svg;
    options.background = ImageExporter::Background::Transparent;
    const QString path = temporaryDir.filePath(QStringLiteral("drawing.svg"));
    QString error;
    QVERIFY2(ImageExporter::save(page, {}, options, path, &error), qPrintable(error));
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray saved = file.readAll();
    QVERIFY(saved.startsWith("<?xml"));
    QVERIFY(saved.contains("id=\"rectangle-1\""));
}

QTEST_MAIN(ImageExporterTest)

#include "tst_imageexporter.moc"
