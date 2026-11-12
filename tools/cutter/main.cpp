#include <QBuffer>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>
#include <QRect>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringList>
#include <QTextStream>
#include <QVector>

struct Parameter {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int cut = 0;
    int cutTop = 0;
    int cutBottom = 0;
    int cutLeft = 0;
    int cutRight = 0;
    QString fileName;
    bool isX = false;
    bool isY = false;
    bool isWidth = false;
    bool isHeight = false;
    bool isCut = false;
    bool isCutTop = false;
    bool isCutBottom = false;
    bool isCutLeft = false;
    bool isCutRight = false;
    bool clipGreen = false;
    bool overwrite = false;
};

struct GreenClipResult {
    QRect rect;
    QVector<uchar> background;
    int imageWidth = 0;
    int imageHeight = 0;
};

static void usage()
{
    const auto programName = QFileInfo(QCoreApplication::arguments().at(0)).baseName();
    QTextStream out(stderr);
    out
        << "Usage: " << programName
        << " [--overwrite]" << Qt::endl
        << "       [--clip-green | --cut=N | --cut-top=N | --cut-bottom=N | --cut-left=N | --cut-right=N]" << Qt::endl
        << "       [GEOMETRY | --geometry=GEOMETRY] FILE" << Qt::endl
        << Qt::endl
        << "Options:" << Qt::endl
        << "  --overwrite      overwrite output file without asking" << Qt::endl
        << "  --clip-green     crop away edge-connected chroma green background" << Qt::endl
        << "  --cut=N          cut N pixels from all sides" << Qt::endl
        << "  --cut-top=N      cut N pixels from the top edge" << Qt::endl
        << "  --cut-bottom=N   cut N pixels from the bottom edge" << Qt::endl
        << "  --cut-left=N     cut N pixels from the left edge" << Qt::endl
        << "  --cut-right=N    cut N pixels from the right edge" << Qt::endl
        << Qt::endl
        << "GEOMETRY formats:" << Qt::endl
        << "  WxH+X+Y        width/height with offset" << Qt::endl
        << "  X1,Y1:X2,Y2    top-left and bottom-right corners" << Qt::endl
        << "  X1,Y1-X2,Y2    same as above" << Qt::endl
        << Qt::endl
        << "Notes:" << Qt::endl
        << "  --clip-green cannot be used together with cut options or GEOMETRY." << Qt::endl
        << "  Cut options cannot be used together with GEOMETRY." << Qt::endl
        << "  --cut cannot be used together with individual cut-edge options." << Qt::endl
        << Qt::endl
        << "Examples:" << Qt::endl
        << "  " << programName << " photo.jpg" << Qt::endl
        << "  " << programName << " --clip-green input.png" << Qt::endl
        << "  " << programName << " --cut=5 input.png" << Qt::endl
        << "  " << programName << " --cut-top=5 input.png" << Qt::endl
        << "  " << programName << " --cut-top=5 --cut-right=10 input.png" << Qt::endl
        << "  " << programName << " --overwrite 100x100+10+20 input.png" << Qt::endl
        << "  " << programName << " --over 100x100+10+20 input.png" << Qt::endl
        << "  " << programName << " 10,20:110,120 input.png" << Qt::endl
        << "  " << programName << " --geometry=10,20-110,120 input.png" << Qt::endl;
}

static bool parseNonNegativeInt(const QString& value, int& result)
{
    bool ok = false;
    result = value.toInt(&ok);
    return ok && result >= 0;
}

static bool parseGeometry(const QString& geometry, Parameter& parameter)
{
    if (geometry.isEmpty()) {
        return false;
    }

    const QRegularExpression cornersPattern(
        QStringLiteral("^(-?\\d+),(-?\\d+)(?:[:-])(-?\\d+),(-?\\d+)$"));
    const QRegularExpressionMatch cornersMatch = cornersPattern.match(geometry);
    if (cornersMatch.hasMatch()) {
        const int x1 = cornersMatch.captured(1).toInt();
        const int y1 = cornersMatch.captured(2).toInt();
        const int x2 = cornersMatch.captured(3).toInt();
        const int y2 = cornersMatch.captured(4).toInt();
        if (x2 <= x1 || y2 <= y1)
            return false;

        parameter.x = x1;
        parameter.y = y1;
        parameter.width = x2 - x1;
        parameter.height = y2 - y1;
        parameter.isX = true;
        parameter.isY = true;
        parameter.isWidth = true;
        parameter.isHeight = true;
        return true;
    }

    QString w, h, x, y;
    const int xPos = geometry.indexOf('x');
    int offsetPos = 0;
    if (xPos >= 0) {
        w = geometry.mid(0, xPos);
        int pos = geometry.indexOf(QRegularExpression("[^0-9]"), xPos + 1);
        if (pos > 0) {
            h = geometry.mid(xPos + 1, pos - (xPos + 1));
            offsetPos = pos;
        } else {
            offsetPos = geometry.length();
        }
    }

    if (offsetPos < geometry.length()) {
        if (geometry[offsetPos] == '-' || geometry[offsetPos] == '+') {
            int pos = geometry.indexOf(QRegularExpression("[^0-9]"), offsetPos + 1);
            if (pos > 0) {
                x = geometry.mid(offsetPos, pos - offsetPos);
                y = geometry.right(geometry.length() - pos);
            } else {
                x = geometry.right(geometry.length() - offsetPos);
            }
        } else {
            w = geometry;
        }
    }

    if (!w.isEmpty()) {
        parameter.width = w.toInt(&parameter.isWidth);
        if (!parameter.isWidth)
            return false;
    }

    if (!h.isEmpty()) {
        parameter.height = h.toInt(&parameter.isHeight);
        if (!parameter.isHeight)
            return false;
    }

    if (!x.isEmpty()) {
        parameter.x = x.toInt(&parameter.isX);
        if (!parameter.isX)
            return false;
    }

    if (!y.isEmpty()) {
        parameter.y = y.toInt(&parameter.isY);
        if (!parameter.isY)
            return false;
    }

    return true;
}

static bool parseCommandLine(Parameter& parameter)
{
    bool fileNameSet = false;
    const auto arguments = QCoreApplication::arguments();

    for (int n = 1; n < arguments.count(); ++n) {
        const auto param = arguments.at(n);
        if (param.startsWith("--")) {
            if (QStringLiteral("--overwrite").startsWith(param)) {
                parameter.overwrite = true;
                continue;
            }
            if (param == "--clip-green") {
                parameter.clipGreen = true;
                continue;
            }
            QStringList splittedParam = param.split('=');
            if (splittedParam.count() == 2) {
                if (QStringLiteral("--geometry").startsWith(splittedParam.at(0))) {
                    if (!parseGeometry(splittedParam.at(1), parameter))
                        return false;
                } else if (splittedParam.at(0) == "--cut") {
                    if (!parseNonNegativeInt(splittedParam.at(1), parameter.cut))
                        return false;
                    parameter.isCut = true;
                } else if (splittedParam.at(0) == "--cut-top") {
                    if (!parseNonNegativeInt(splittedParam.at(1), parameter.cutTop))
                        return false;
                    parameter.isCutTop = true;
                } else if (splittedParam.at(0) == "--cut-bottom") {
                    if (!parseNonNegativeInt(splittedParam.at(1), parameter.cutBottom))
                        return false;
                    parameter.isCutBottom = true;
                } else if (splittedParam.at(0) == "--cut-left") {
                    if (!parseNonNegativeInt(splittedParam.at(1), parameter.cutLeft))
                        return false;
                    parameter.isCutLeft = true;
                } else if (splittedParam.at(0) == "--cut-right") {
                    if (!parseNonNegativeInt(splittedParam.at(1), parameter.cutRight))
                        return false;
                    parameter.isCutRight = true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (param.startsWith('-')) {
            return false;
        } else if (!fileNameSet && parseGeometry(param, parameter)) {
            continue;
        } else if (!fileNameSet) {
            parameter.fileName = param;
            fileNameSet = true;
        } else {
            return false;
        }
    }

    const bool hasGeometry =
        parameter.isX || parameter.isY || parameter.isWidth || parameter.isHeight;
    const bool hasCutEdgeOption =
        parameter.isCutTop || parameter.isCutBottom || parameter.isCutLeft || parameter.isCutRight;

    if (parameter.clipGreen && (hasGeometry || parameter.isCut || hasCutEdgeOption))
        return false;

    if (hasGeometry && (parameter.isCut || hasCutEdgeOption))
        return false;

    if (parameter.isCut && hasCutEdgeOption)
        return false;

    return true;
}

static bool confirmOverwrite(const QString& fileName)
{
    QTextStream out(stdout);
    QTextStream in(stdin);

    out << fileName << " already exists. Overwrite? [y/N] " << Qt::flush;
    const QString answer = in.readLine().trimmed().toLower();
    return answer == "y" || answer == "yes";
}

static QString normalizedOutputSuffix(const QString& fileName)
{
    const QString suffix = QFileInfo(fileName).suffix().toLower();

    if (suffix == "jpeg")
        return "jpg";

    return suffix;
}

static QString makeOutputFileName(const QString& inputFileName,
                                  const QByteArray& sizeNote,
                                  const QString& suffix)
{
    const QString normalizedSuffix = suffix.isEmpty() ? QStringLiteral("png") : suffix;
    return QFileInfo(inputFileName).completeBaseName() + "-" + sizeNote + "." + normalizedSuffix;
}

static bool isGreenClipBackground(QRgb pixel)
{
    constexpr int alphaThreshold = 8;

    const int red = qRed(pixel);
    const int green = qGreen(pixel);
    const int blue = qBlue(pixel);
    const int alpha = qAlpha(pixel);

    if (alpha <= alphaThreshold)
        return true;

    const bool isChromaGreen = green >= 80 && green - qMax(red, blue) >= 35;

    return isChromaGreen;
}

static QRgb removeGreenFringe(QRgb pixel)
{
    constexpr int alphaThreshold = 8;

    const int red = qRed(pixel);
    const int green = qGreen(pixel);
    const int blue = qBlue(pixel);
    const int alpha = qAlpha(pixel);
    const int maxRedBlue = qMax(red, blue);

    if (alpha <= alphaThreshold || green < 45 || green - maxRedBlue < 12)
        return pixel;

    const int newAlpha = qBound(0, maxRedBlue * 255 / green, alpha);
    return qRgba(red, maxRedBlue, blue, newAlpha);
}

static GreenClipResult greenClipResult(const QImage& image)
{
    constexpr int alphaThreshold = 8;
    const int width = image.width();
    const int height = image.height();
    GreenClipResult result;
    result.imageWidth = width;
    result.imageHeight = height;
    result.background.resize(width * height);
    QVector<QPoint> stack;

    auto index = [width](int x, int y) {
        return y * width + x;
    };

    auto addIfBackground = [&](int x, int y) {
        const int i = index(x, y);
        if (result.background.at(i) || !isGreenClipBackground(image.pixel(x, y)))
            return;

        result.background[i] = 1;
        stack.append(QPoint(x, y));
    };

    for (int x = 0; x < width; ++x) {
        addIfBackground(x, 0);
        addIfBackground(x, height - 1);
    }
    for (int y = 0; y < height; ++y) {
        addIfBackground(0, y);
        addIfBackground(width - 1, y);
    }

    while (!stack.isEmpty()) {
        const QPoint point = stack.takeLast();
        const int x = point.x();
        const int y = point.y();

        if (x > 0)
            addIfBackground(x - 1, y);
        if (x < width - 1)
            addIfBackground(x + 1, y);
        if (y > 0)
            addIfBackground(x, y - 1);
        if (y < height - 1)
            addIfBackground(x, y + 1);
    }

    int minX = image.width();
    int minY = image.height();
    int maxX = -1;
    int maxY = -1;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const QRgb pixel = image.pixel(x, y);
            if (qAlpha(pixel) <= alphaThreshold || result.background.at(index(x, y)))
                continue;

            minX = qMin(minX, x);
            minY = qMin(minY, y);
            maxX = qMax(maxX, x);
            maxY = qMax(maxY, y);
        }
    }

    if (maxX < minX || maxY < minY)
        return result;

    result.rect = QRect(QPoint(minX, minY), QPoint(maxX, maxY));
    return result;
}

static QImage makeGreenClippedImage(const QImage& image,
                                    const GreenClipResult& result)
{
    QImage transparent = image.convertToFormat(QImage::Format_ARGB32);

    auto index = [width = result.imageWidth](int x, int y) {
        return y * width + x;
    };

    for (int y = 0; y < transparent.height(); ++y) {
        for (int x = 0; x < transparent.width(); ++x) {
            QRgb pixel = transparent.pixel(x, y);
            if (qAlpha(pixel) <= 8 || result.background.at(index(x, y)))
                transparent.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), 0));
        }
    }

    QImage cropped = transparent.copy(result.rect);
    for (int y = 0; y < cropped.height(); ++y) {
        for (int x = 0; x < cropped.width(); ++x) {
            const QRgb pixel = cropped.pixel(x, y);
            cropped.setPixel(x, y, removeGreenFringe(pixel));
        }
    }

    QImage output(cropped.width() * 2, cropped.height() * 2, QImage::Format_ARGB32);
    output.fill(qRgba(0, 0, 0, 0));

    const int offsetX = cropped.width() / 2;
    const int offsetY = cropped.height() / 2;
    for (int y = 0; y < cropped.height(); ++y) {
        for (int x = 0; x < cropped.width(); ++x) {
            const QRgb pixel = cropped.pixel(x, y);
            if (qAlpha(pixel) > 8)
                output.setPixel(x + offsetX, y + offsetY, pixel);
        }
    }

    return output;
}

static bool saveImageWithFallback(const QImage& image,
                                  const QString& inputFileName,
                                  const QByteArray& sizeNote,
                                  bool overwrite,
                                  QString& savedFileName)
{
    const QList<QByteArray> supportedFormats = QImageWriter::supportedImageFormats();
    const QString preferredSuffix = normalizedOutputSuffix(inputFileName);
    const QByteArray preferredFormat = preferredSuffix.toLatin1();
    const bool canTryPreferredFormat =
        !preferredSuffix.isEmpty() && supportedFormats.contains(preferredFormat);

    auto saveWithFormat = [&](const QString& suffix, const char* format) -> bool {
        const QString outputFileName = makeOutputFileName(inputFileName, sizeNote, suffix);
        if (QFile::exists(outputFileName)
            && !overwrite
            && !confirmOverwrite(outputFileName)) {
            return false;
        }

        if (!image.save(outputFileName, format))
            return false;

        savedFileName = outputFileName;
        return true;
    };

    if (canTryPreferredFormat && saveWithFormat(preferredSuffix, preferredFormat.constData()))
        return true;

    const QString pngFileName = makeOutputFileName(inputFileName, sizeNote, QStringLiteral("png"));
    QString fallbackReason;

    if (canTryPreferredFormat) {
        fallbackReason = QStringLiteral("Cannot save in .%1").arg(preferredSuffix);
    } else if (!preferredSuffix.isEmpty()) {
        fallbackReason = QStringLiteral("Format .%1 is not supported for output").arg(preferredSuffix);
    } else {
        fallbackReason = QStringLiteral("Input file has no output extension");
    }

    if (saveWithFormat(QStringLiteral("png"), "PNG")) {
        QTextStream(stderr) << fallbackReason
                            << ". Saved as PNG: " << pngFileName << Qt::endl;
        return true;
    }

    QTextStream(stderr) << fallbackReason
                        << ". PNG fallback also failed." << Qt::endl;
    return false;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Parameter parameter;
    if (!parseCommandLine(parameter) || parameter.fileName.isEmpty()) {
        usage();
        return -1;
    }

    QImage sourceImage(parameter.fileName);

    if (sourceImage.isNull()) {
        QTextStream(stderr) << "Cannot read " << parameter.fileName << Qt::endl;
        return -1;
    }

    QRect cropRect;
    GreenClipResult greenClip;
    if (parameter.clipGreen) {
        greenClip = greenClipResult(sourceImage);
        cropRect = greenClip.rect;
        if (cropRect.isEmpty()) {
            QTextStream(stderr) << "No foreground pixels found" << Qt::endl;
            return -1;
        }
    } else {
        int x = 0;
        if (parameter.isX)
            x = parameter.x;

        int y = 0;
        if (parameter.isY)
            y = parameter.y;

        int width = sourceImage.size().width();
        if (parameter.isWidth)
            width = parameter.width;

        int height = sourceImage.size().height();
        if (parameter.isHeight)
            height = parameter.height;

        if (parameter.isCut) {
            x += parameter.cut;
            y += parameter.cut;
            width -= parameter.cut * 2;
            height -= parameter.cut * 2;
        } else {
            if (parameter.isCutLeft) {
                x += parameter.cutLeft;
                width -= parameter.cutLeft;
            }
            if (parameter.isCutRight)
                width -= parameter.cutRight;
            if (parameter.isCutTop) {
                y += parameter.cutTop;
                height -= parameter.cutTop;
            }
            if (parameter.isCutBottom)
                height -= parameter.cutBottom;
        }

        cropRect = QRect(x, y, width, height);
        cropRect = cropRect.intersected(sourceImage.rect());
    }

    if (cropRect.isEmpty())
        return -1;

    const int x = cropRect.x();
    const int y = cropRect.y();
    QImage outImage = parameter.clipGreen
        ? makeGreenClippedImage(sourceImage, greenClip)
        : sourceImage.copy(cropRect);
    if (outImage.isNull())
        return -1;

    const int width = outImage.width();
    const int height = outImage.height();

    QByteArray sizeNote;
    QBuffer sizeNoteBuffer(&sizeNote);
    sizeNoteBuffer.open(QIODevice::WriteOnly);
    QTextStream(&sizeNoteBuffer) << width << "x" << height
                                 << QString(x >= 0 ? "+" : "") << x
                                 << QString(y >= 0 ? "+" : "") << y;
    sizeNoteBuffer.close();

    QString outputFileName;
    if (!saveImageWithFallback(outImage, parameter.fileName, sizeNote,
                               parameter.overwrite, outputFileName))
        return -1;

    QTextStream(stdout) << outputFileName << Qt::endl;

    return 0;
}
