#include "ImageExporter.h"

#include "BoardEnums.h"
#include "DrawingRenderer.h"

#include <QFileInfo>
#include <QFontMetricsF>
#include <QLineF>
#include <QPainter>
#include <QPainterPath>
#include <QSaveFile>
#include <QSet>
#include <QTransform>
#include <QtMath>

#include <cmath>

namespace {
QString number(qreal value)
{
    if (qFuzzyIsNull(value))
        value = 0.0;
    return QString::number(value, 'g', 12);
}

QString escaped(const QString &value)
{
    return value.toHtmlEscaped();
}

QString colorValue(const QColor &color)
{
    return color.name(QColor::HexRgb);
}

QString opacityAttribute(const QString &name, const QColor &color)
{
    if (color.alpha() >= 255)
        return QString();
    return QStringLiteral(" %1=\"%2\"").arg(name, number(color.alphaF()));
}

bool usesPointGeometry(ElementType type)
{
    return type == ElementType::Freehand || type == ElementType::Line
        || type == ElementType::Polyline || type == ElementType::Bezier;
}

bool supportsFillColor(const DrawingElement &element)
{
    if (element.type() == ElementType::Rectangle || element.type() == ElementType::RoundedRectangle
        || element.type() == ElementType::Ellipse || element.type() == ElementType::Circle) {
        return true;
    }
    if (element.type() == ElementType::Bezier)
        return element.closed();
    return element.type() == ElementType::Polyline && element.closed();
}

bool supportsArrowHeads(const DrawingElement &element)
{
    if (element.type() == ElementType::Line || element.type() == ElementType::Arc)
        return true;
    if (element.type() == ElementType::Bezier)
        return !element.closed();
    return element.type() == ElementType::Polyline && !element.closed();
}

QVector<int> exportIndexes(const Page &page, const QVector<int> &selectedIndexes)
{
    const QVector<DrawingElement> elements = page.elements();
    if (selectedIndexes.isEmpty()) {
        QVector<int> indexes;
        indexes.reserve(elements.size());
        for (int i = 0; i < elements.size(); ++i)
            indexes.append(i);
        return indexes;
    }

    const QSet<int> selected(selectedIndexes.cbegin(), selectedIndexes.cend());
    QVector<int> indexes;
    indexes.reserve(selected.size());
    for (int i = 0; i < elements.size(); ++i) {
        if (selected.contains(i))
            indexes.append(i);
    }
    return indexes;
}

QRectF geometryBounds(const Page &page, const QVector<int> &indexes)
{
    const QVector<DrawingElement> elements = page.elements();
    QRectF bounds;
    bool hasBounds = false;
    for (int index : indexes) {
        if (index < 0 || index >= elements.size())
            continue;
        const QRectF elementBounds = elements.at(index).visualBounds();
        bounds = hasBounds ? bounds.united(elementBounds) : elementBounds;
        hasBounds = true;
    }
    return hasBounds ? bounds.normalized() : QRectF();
}

qreal scratchMargin(const Page &page, const QVector<int> &indexes)
{
    const QVector<DrawingElement> elements = page.elements();
    qreal margin = 2.0;
    for (int index : indexes) {
        if (index < 0 || index >= elements.size())
            continue;
        const DrawingElement &element = elements.at(index);
        const qreal strokeMargin = qMax<qreal>(1.0, element.strokeWidth() / 2.0 + 2.0);
        const qreal arrowMargin = supportsArrowHeads(element)
            ? qMax<qreal>(18.0, element.strokeWidth() * 6.0 + 2.0) : 0.0;
        margin = qMax(margin, qMax(strokeMargin, arrowMargin));
    }
    return margin;
}

void drawElements(QPainter &painter, const Page &page, const QVector<int> &indexes)
{
    const QVector<DrawingElement> elements = page.elements();
    for (int index : indexes) {
        if (index >= 0 && index < elements.size())
            DrawingRenderer::drawElement(painter, elements.at(index));
    }
}

QRect paintedBounds(const Page &page, const QVector<int> &indexes)
{
    const QRectF rawBounds = geometryBounds(page, indexes);
    if (indexes.isEmpty() || !rawBounds.isValid())
        return QRect();

    const qreal margin = scratchMargin(page, indexes);
    const QRect scratchRect = rawBounds.adjusted(-margin, -margin, margin, margin).toAlignedRect();
    if (scratchRect.isEmpty())
        return QRect();

    QImage scratch(scratchRect.size(), QImage::Format_ARGB32_Premultiplied);
    scratch.fill(Qt::transparent);
    QPainter painter(&scratch);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(-scratchRect.left(), -scratchRect.top());
    drawElements(painter, page, indexes);
    painter.end();

    int minX = scratch.width();
    int minY = scratch.height();
    int maxX = -1;
    int maxY = -1;
    for (int y = 0; y < scratch.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(scratch.constScanLine(y));
        for (int x = 0; x < scratch.width(); ++x) {
            if (qAlpha(line[x]) == 0)
                continue;
            minX = qMin(minX, x);
            minY = qMin(minY, y);
            maxX = qMax(maxX, x);
            maxY = qMax(maxY, y);
        }
    }

    if (maxX < minX || maxY < minY)
        return rawBounds.toAlignedRect().adjusted(-1, -1, 1, 1);

    const QRect pixels(QPoint(minX, minY), QPoint(maxX, maxY));
    return pixels.translated(scratchRect.topLeft()).adjusted(-1, -1, 1, 1);
}

QString fallbackElementId(ElementType type, int pageIndex)
{
    QString typeName = elementTypeToString(type);
    for (int i = 1; i < typeName.size(); ++i) {
        if (typeName.at(i).isUpper()) {
            typeName.insert(i, QLatin1Char('-'));
            ++i;
        }
    }
    return typeName.toLower() + QLatin1Char('-') + QString::number(pageIndex + 1);
}

QString svgElementId(const DrawingElement &element, int pageIndex)
{
    const QString id = element.id().trimmed();
    return id.isEmpty() ? fallbackElementId(element.type(), pageIndex) : id;
}

QString pointList(const QVector<QPointF> &points)
{
    QStringList values;
    values.reserve(points.size());
    for (const QPointF &point : points)
        values.append(number(point.x()) + QLatin1Char(',') + number(point.y()));
    return values.join(QLatin1Char(' '));
}

QString painterPathData(const QPainterPath &path, bool closePath = false)
{
    QString data;
    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element element = path.elementAt(i);
        if (element.isMoveTo()) {
            data += QStringLiteral("M %1 %2 ").arg(number(element.x), number(element.y));
        } else if (element.isLineTo()) {
            data += QStringLiteral("L %1 %2 ").arg(number(element.x), number(element.y));
        } else if (element.type == QPainterPath::CurveToElement && i + 2 < path.elementCount()) {
            const QPainterPath::Element control2 = path.elementAt(i + 1);
            const QPainterPath::Element end = path.elementAt(i + 2);
            data += QStringLiteral("C %1 %2 %3 %4 %5 %6 ")
                        .arg(number(element.x), number(element.y),
                             number(control2.x), number(control2.y),
                             number(end.x), number(end.y));
            i += 2;
        }
    }
    if (closePath)
        data += QLatin1Char('Z');
    return data.trimmed();
}

QString transformAttribute(const DrawingElement &element)
{
    if (usesPointGeometry(element.type()) || element.type() == ElementType::Circle
        || qFuzzyIsNull(element.rotationDegrees())) {
        return QString();
    }
    const QPointF center = element.bounds().center();
    return QStringLiteral(" transform=\"rotate(%1 %2 %3)\"")
        .arg(number(element.rotationDegrees()), number(center.x()), number(center.y()));
}

QString paintAttributes(const DrawingElement &element)
{
    QString attributes;
    if (element.strokeWidth() > 0) {
        attributes += QStringLiteral(" stroke=\"%1\" stroke-width=\"%2\"")
                          .arg(colorValue(element.color()), number(element.strokeWidth()));
        attributes += opacityAttribute(QStringLiteral("stroke-opacity"), element.color());
        attributes += QStringLiteral(" stroke-linecap=\"round\" stroke-linejoin=\"round\"");
        if (element.strokeStyle() == StrokeStyle::Dotted) {
            attributes += QStringLiteral(" stroke-dasharray=\"%1 %2\"")
                              .arg(number(qMax(1, element.strokeWidth())),
                                   number(qMax(2, element.strokeWidth() * 2)));
        }
    } else {
        attributes += QStringLiteral(" stroke=\"none\"");
    }

    const QColor fill = element.fillColor();
    if (supportsFillColor(element) && fill.alpha() > 0) {
        attributes += QStringLiteral(" fill=\"%1\"").arg(colorValue(fill));
        attributes += opacityAttribute(QStringLiteral("fill-opacity"), fill);
    } else {
        attributes += QStringLiteral(" fill=\"none\"");
    }
    return attributes;
}

QPointF pointOnEllipse(const QRectF &rect, int angleDegrees)
{
    const qreal radians = qDegreesToRadians(static_cast<qreal>(angleDegrees));
    const QPointF center = rect.center();
    return QPointF(
        center.x() + std::cos(radians) * rect.width() / 2.0,
        center.y() - std::sin(radians) * rect.height() / 2.0);
}

QString svgArrowHead(const QPointF &tip,
                     const QPointF &tail,
                     ArrowHead arrowHead,
                     const QColor &color,
                     qreal strokeWidth)
{
    if (arrowHead == ArrowHead::None || strokeWidth <= 0)
        return QString();
    QLineF direction(tail, tip);
    if (direction.length() <= 0.5)
        return QString();
    direction.setLength(qMax<qreal>(12.0, strokeWidth * 4.0));

    const qreal angle = std::atan2(tip.y() - tail.y(), tip.x() - tail.x());
    const qreal sideAngle = qDegreesToRadians(28.0);
    const qreal length = direction.length();
    const QPointF left(tip.x() - std::cos(angle - sideAngle) * length,
                       tip.y() - std::sin(angle - sideAngle) * length);
    const QPointF right(tip.x() - std::cos(angle + sideAngle) * length,
                        tip.y() - std::sin(angle + sideAngle) * length);
    const QString stroke = QStringLiteral(" stroke=\"%1\" stroke-width=\"%2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"")
                               .arg(colorValue(color), number(strokeWidth))
        + opacityAttribute(QStringLiteral("stroke-opacity"), color);
    if (arrowHead == ArrowHead::Open) {
        return QStringLiteral("<path d=\"M %1 %2 L %3 %4 M %1 %2 L %5 %6\"%7 fill=\"none\"/>")
            .arg(number(tip.x()), number(tip.y()), number(left.x()), number(left.y()),
                 number(right.x()), number(right.y()), stroke);
    }
    if (arrowHead == ArrowHead::Triangle) {
        return QStringLiteral("<polygon points=\"%1\"%2 fill=\"%3\"%4/>")
            .arg(pointList({tip, left, right}), stroke, colorValue(color),
                 opacityAttribute(QStringLiteral("fill-opacity"), color));
    }
    const QPointF back(tip.x() - std::cos(angle) * length * 1.35,
                       tip.y() - std::sin(angle) * length * 1.35);
    return QStringLiteral("<polygon points=\"%1\"%2 fill=\"none\"/>")
        .arg(pointList({tip, left, back, right}), stroke);
}

QString svgArrowHeads(const DrawingElement &element)
{
    if (!supportsArrowHeads(element) || element.strokeWidth() <= 0)
        return QString();
    QPointF startTip;
    QPointF startTail;
    QPointF endTip;
    QPointF endTail;
    if (element.type() == ElementType::Line || element.type() == ElementType::Polyline
        || element.type() == ElementType::Bezier) {
        const QVector<QPointF> points = element.points();
        if (points.size() < 2)
            return QString();
        startTip = points.first();
        startTail = points.at(1);
        endTip = points.last();
        endTail = points.at(points.size() - 2);
    } else {
        const QRectF rect = element.bounds();
        const int start = element.arcStartAngle();
        const int end = start + element.arcSpanAngle();
        const int step = element.arcSpanAngle() >= 0 ? 4 : -4;
        startTip = pointOnEllipse(rect, start);
        startTail = pointOnEllipse(rect, start + step);
        endTip = pointOnEllipse(rect, end);
        endTail = pointOnEllipse(rect, end - step);
    }
    return svgArrowHead(startTip, startTail, element.startArrowHead(), element.color(), element.strokeWidth())
        + svgArrowHead(endTip, endTail, element.endArrowHead(), element.color(), element.strokeWidth());
}

QString svgText(const DrawingElement &element)
{
    const QFont font = element.font();
    const qreal fontSize = font.pointSizeF() > 0 ? font.pointSizeF() : qMax(1, font.pixelSize());
    const QFontMetricsF metrics(font);
    const QRectF rect = element.rect().normalized();
    const QStringList lines = element.text().split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    QString text = QStringLiteral("<text x=\"%1\" y=\"%2\" fill=\"%3\"%4 font-family=\"%5\" font-size=\"%6pt\" font-weight=\"%7\" font-style=\"%8\" xml:space=\"preserve\">")
                       .arg(number(rect.left()), number(rect.top() + metrics.ascent()),
                            colorValue(element.color()), opacityAttribute(QStringLiteral("fill-opacity"), element.color()),
                            escaped(font.family()), number(fontSize), font.bold() ? QStringLiteral("700") : QStringLiteral("400"),
                            font.italic() ? QStringLiteral("italic") : QStringLiteral("normal"));
    for (int i = 0; i < lines.size(); ++i) {
        const QString content = lines.at(i).isEmpty() ? QStringLiteral("&#160;") : escaped(lines.at(i));
        text += QStringLiteral("<tspan x=\"%1\" dy=\"%2\">%3</tspan>")
                    .arg(number(rect.left()), number(i == 0 ? 0.0 : metrics.lineSpacing()), content);
    }
    text += QStringLiteral("</text>");
    return text;
}

QString svgImage(const DrawingElement &element)
{
    if (element.imageData().isEmpty())
        return QString();
    const QRectF rect = element.rect().normalized();
    const QString mimeType = element.imageMimeType().isEmpty()
        ? QStringLiteral("image/png")
        : element.imageMimeType();
    return QStringLiteral("<image x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" href=\"data:%5;base64,%6\"/>")
        .arg(number(rect.left()), number(rect.top()), number(rect.width()), number(rect.height()),
             mimeType, QString::fromLatin1(element.imageData().toBase64()));
}

QString svgElement(const DrawingElement &element, int pageIndex)
{
    const QString id = svgElementId(element, pageIndex);
    QString body;
    const QString paint = paintAttributes(element);
    switch (element.type()) {
    case ElementType::Freehand: {
        QPainterPath path;
        if (!element.points().isEmpty()) {
            path.moveTo(element.points().first());
            for (int i = 1; i < element.points().size(); ++i)
                path.lineTo(element.points().at(i));
        }
        body = QStringLiteral("<path d=\"%1\"%2/>").arg(painterPathData(path), paint);
        break;
    }
    case ElementType::Line: {
        const QVector<QPointF> points = element.points();
        if (points.size() >= 2) {
            body = QStringLiteral("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\"%5/>")
                       .arg(number(points.at(0).x()), number(points.at(0).y()),
                            number(points.at(1).x()), number(points.at(1).y()), paint);
        }
        break;
    }
    case ElementType::Polyline:
        body = QStringLiteral("<%1 points=\"%2\"%3/>")
                   .arg(element.closed() ? QStringLiteral("polygon") : QStringLiteral("polyline"),
                        pointList(element.points()), paint);
        break;
    case ElementType::Bezier:
        body = QStringLiteral("<path d=\"%1\"%2/>")
                   .arg(painterPathData(DrawingRenderer::bezierPath(element.points(), element.closed()), element.closed()), paint);
        break;
    case ElementType::Rectangle: {
        const QRectF rect = element.rect().normalized();
        body = QStringLiteral("<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\"%5/>")
                   .arg(number(rect.left()), number(rect.top()), number(rect.width()), number(rect.height()), paint);
        break;
    }
    case ElementType::RoundedRectangle: {
        const QRectF rect = element.rect().normalized();
        const qreal radius = qMin<qreal>(qMax(0, element.cornerRadius()), qMin(rect.width(), rect.height()) / 2.0);
        body = QStringLiteral("<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" rx=\"%5\" ry=\"%5\"%6/>")
                   .arg(number(rect.left()), number(rect.top()), number(rect.width()), number(rect.height()), number(radius), paint);
        break;
    }
    case ElementType::Ellipse:
    case ElementType::Circle: {
        const QRectF rect = element.type() == ElementType::Circle ? element.bounds() : element.rect().normalized();
        body = QStringLiteral("<ellipse cx=\"%1\" cy=\"%2\" rx=\"%3\" ry=\"%4\"%5/>")
                   .arg(number(rect.center().x()), number(rect.center().y()),
                        number(rect.width() / 2.0), number(rect.height() / 2.0), paint);
        break;
    }
    case ElementType::Arc:
        body = QStringLiteral("<path d=\"%1\"%2/>")
                   .arg(painterPathData(DrawingRenderer::arcPath(element.bounds(), element.arcStartAngle(), element.arcSpanAngle())), paint);
        break;
    case ElementType::Text:
        body = svgText(element);
        break;
    case ElementType::Image:
        body = svgImage(element);
        break;
    }
    body += svgArrowHeads(element);
    return QStringLiteral("<g id=\"%1\"%2>%3</g>\n")
        .arg(escaped(id), transformAttribute(element), body);
}
}

bool ImageExporter::hasExportableElements(const Page &page)
{
    return !page.elements().isEmpty();
}

QRect ImageExporter::exportBounds(const Page &page, const QVector<int> &selectedIndexes)
{
    return paintedBounds(page, exportIndexes(page, selectedIndexes));
}

QImage ImageExporter::renderPng(const Page &page,
                                const QVector<int> &selectedIndexes,
                                Background background,
                                QRect *bounds)
{
    const QVector<int> indexes = exportIndexes(page, selectedIndexes);
    const QRect targetBounds = paintedBounds(page, indexes);
    if (bounds)
        *bounds = targetBounds;
    if (targetBounds.isEmpty())
        return QImage();

    QImage image(targetBounds.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(background == Background::White ? Qt::white : Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(-targetBounds.left(), -targetBounds.top());
    drawElements(painter, page, indexes);
    return image;
}

QByteArray ImageExporter::renderSvg(const Page &page,
                                    const QVector<int> &selectedIndexes,
                                    Background background,
                                    QRect *bounds)
{
    const QVector<int> indexes = exportIndexes(page, selectedIndexes);
    const QRect targetBounds = paintedBounds(page, indexes);
    if (bounds)
        *bounds = targetBounds;
    if (targetBounds.isEmpty())
        return QByteArray();

    QString svg = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                 "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%1\" height=\"%2\" viewBox=\"%3 %4 %1 %2\">\n")
                      .arg(targetBounds.width())
                      .arg(targetBounds.height())
                      .arg(targetBounds.left())
                      .arg(targetBounds.top());
    if (background == Background::White) {
        svg += QStringLiteral("<rect id=\"background\" x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" fill=\"#ffffff\"/>\n")
                   .arg(targetBounds.left()).arg(targetBounds.top())
                   .arg(targetBounds.width()).arg(targetBounds.height());
    }
    const QVector<DrawingElement> elements = page.elements();
    for (int index : indexes) {
        if (index >= 0 && index < elements.size())
            svg += svgElement(elements.at(index), index);
    }
    svg += QStringLiteral("</svg>\n");
    return svg.toUtf8();
}

bool ImageExporter::save(const Page &page,
                         const QVector<int> &selectedIndexes,
                         const Options &options,
                         const QString &filePath,
                         QString *errorMessage)
{
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    bool written = false;
    if (options.format == Format::Png) {
        const QImage image = renderPng(page, selectedIndexes, options.background);
        written = !image.isNull() && image.save(&file, "PNG");
    } else {
        const QByteArray svg = renderSvg(page, selectedIndexes, options.background);
        written = !svg.isEmpty() && file.write(svg) == svg.size();
    }
    if (!written) {
        if (errorMessage)
            *errorMessage = file.errorString().isEmpty() ? QStringLiteral("Could not encode the image.") : file.errorString();
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }
    return true;
}

QString ImageExporter::extension(Format format)
{
    return format == Format::Png ? QStringLiteral("png") : QStringLiteral("svg");
}

QString ImageExporter::withDefaultExtension(const QString &filePath, Format format)
{
    if (!QFileInfo(filePath).suffix().isEmpty())
        return filePath;
    return filePath + QLatin1Char('.') + extension(format);
}

bool ImageExporter::formatForFilePath(const QString &filePath, Format fallback, Format *format)
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix.isEmpty()) {
        if (format)
            *format = fallback;
        return true;
    }
    if (suffix == QStringLiteral("png")) {
        if (format)
            *format = Format::Png;
        return true;
    }
    if (suffix == QStringLiteral("svg")) {
        if (format)
            *format = Format::Svg;
        return true;
    }
    return false;
}
