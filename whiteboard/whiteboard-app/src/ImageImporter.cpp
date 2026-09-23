#include "ImageImporter.h"

#include <QBuffer>
#include <QFileInfo>
#include <QImageReader>
#include <QIODevice>
#include <QPainter>
#include <QRectF>
#include <QSizeF>
#include <QSvgRenderer>

namespace {
constexpr int MaxInitialWidth = 960;
constexpr int MaxInitialHeight = 720;
constexpr int MaxSvgRasterSize = 4096;

bool encodePng(const QImage &image, QByteArray *data)
{
    QByteArray encoded;
    QBuffer buffer(&encoded);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "PNG"))
        return false;
    *data = encoded;
    return true;
}

QSize boundedSize(const QSize &source, const QSize &limit)
{
    QSize size = source;
    if (!size.isValid() || size.isEmpty())
        size = QSize(640, 480);
    if (size.width() > limit.width() || size.height() > limit.height())
        size.scale(limit, Qt::KeepAspectRatio);
    return size.expandedTo(QSize(1, 1));
}

QRectF placedRect(const QSize &imageSize, const QSize &canvasSize, const QPointF &center)
{
    const QSize displaySize = boundedSize(
        imageSize,
        QSize(qMin(MaxInitialWidth, qMax(1, canvasSize.width() - 160)),
              qMin(MaxInitialHeight, qMax(1, canvasSize.height() - 160))));
    QPointF topLeft(center.x() - displaySize.width() / 2.0,
                    center.y() - displaySize.height() / 2.0);
    topLeft.setX(qBound(0.0, topLeft.x(), qMax(0.0, static_cast<qreal>(canvasSize.width() - displaySize.width()))));
    topLeft.setY(qBound(0.0, topLeft.y(), qMax(0.0, static_cast<qreal>(canvasSize.height() - displaySize.height()))));
    return QRectF(topLeft, QSizeF(displaySize));
}

QPointF canvasCenter(const QSize &canvasSize)
{
    return QPointF(canvasSize.width() / 2.0, canvasSize.height() / 2.0);
}

bool renderSvg(const QString &filePath, QImage *image, QString *errorMessage)
{
    QSvgRenderer renderer(filePath);
    if (!renderer.isValid()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("The SVG file could not be read.");
        return false;
    }

    QSize size = renderer.defaultSize();
    if (!size.isValid() || size.isEmpty()) {
        const QRectF viewBox = renderer.viewBoxF();
        size = viewBox.isValid() && !viewBox.isEmpty()
            ? viewBox.size().toSize()
            : QSize(640, 480);
    }
    size = boundedSize(size, QSize(MaxSvgRasterSize, MaxSvgRasterSize));

    QImage rendered(size, QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::transparent);
    QPainter painter(&rendered);
    renderer.render(&painter);
    painter.end();
    *image = rendered;
    return true;
}
}

QString ImageImporter::nameFilter()
{
    return QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp *.svg)");
}

bool ImageImporter::importFromFile(const QString &filePath,
                                   const QSize &canvasSize,
                                   DrawingElement *element,
                                   QString *errorMessage)
{
    return importFromFile(filePath, canvasSize, canvasCenter(canvasSize), element, errorMessage);
}

bool ImageImporter::importFromFile(const QString &filePath,
                                   const QSize &canvasSize,
                                   const QPointF &center,
                                   DrawingElement *element,
                                   QString *errorMessage)
{
    if (!element)
        return false;

    QImage image;
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix == QStringLiteral("svg")) {
        if (!renderSvg(filePath, &image, errorMessage))
            return false;
    } else {
        QImageReader reader(filePath);
        reader.setAutoTransform(true);
        image = reader.read();
        if (image.isNull()) {
            if (errorMessage)
                *errorMessage = reader.errorString().isEmpty()
                    ? QStringLiteral("The image file could not be read.")
                    : reader.errorString();
            return false;
        }
    }

    return importFromImage(image, canvasSize, center, element, errorMessage);
}

bool ImageImporter::importFromImage(const QImage &image,
                                    const QSize &canvasSize,
                                    DrawingElement *element,
                                    QString *errorMessage)
{
    return importFromImage(image, canvasSize, canvasCenter(canvasSize), element, errorMessage);
}

bool ImageImporter::importFromImage(const QImage &image,
                                    const QSize &canvasSize,
                                    const QPointF &center,
                                    DrawingElement *element,
                                    QString *errorMessage)
{
    if (!element)
        return false;
    if (image.isNull()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("The image could not be read.");
        return false;
    }

    QByteArray data;
    if (!encodePng(image, &data)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("The image could not be encoded.");
        return false;
    }

    DrawingElement imported;
    imported.setType(ElementType::Image);
    imported.setRect(placedRect(image.size(), canvasSize, center));
    imported.setImageData(data);
    imported.setImageMimeType(QStringLiteral("image/png"));
    *element = imported;
    return true;
}
