#pragma once

#include "Page.h"

#include <QByteArray>
#include <QImage>
#include <QRect>
#include <QString>
#include <QVector>

class ImageExporter {
public:
    enum class Format {
        Png,
        Svg
    };

    enum class Background {
        Transparent,
        White
    };

    struct Options {
        Format format = Format::Png;
        Background background = Background::Transparent;
    };

    static bool hasExportableElements(const Page &page);
    static QRect exportBounds(const Page &page, const QVector<int> &selectedIndexes = {});
    static QImage renderPng(const Page &page,
                            const QVector<int> &selectedIndexes,
                            Background background,
                            QRect *bounds = nullptr);
    static QByteArray renderSvg(const Page &page,
                                const QVector<int> &selectedIndexes,
                                Background background,
                                QRect *bounds = nullptr);
    static bool save(const Page &page,
                     const QVector<int> &selectedIndexes,
                     const Options &options,
                     const QString &filePath,
                     QString *errorMessage = nullptr);

    static QString extension(Format format);
    static QString withDefaultExtension(const QString &filePath, Format format);
    static bool formatForFilePath(const QString &filePath, Format fallback, Format *format);
};
