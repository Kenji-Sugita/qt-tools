#pragma once

#include "DrawingElement.h"

#include <QImage>
#include <QPointF>
#include <QString>
#include <QSize>

class ImageImporter {
public:
    static QString nameFilter();
    static bool importFromFile(const QString &filePath,
                               const QSize &canvasSize,
                               DrawingElement *element,
                               QString *errorMessage = nullptr);
    static bool importFromFile(const QString &filePath,
                               const QSize &canvasSize,
                               const QPointF &center,
                               DrawingElement *element,
                               QString *errorMessage = nullptr);
    static bool importFromImage(const QImage &image,
                                const QSize &canvasSize,
                                DrawingElement *element,
                                QString *errorMessage = nullptr);
    static bool importFromImage(const QImage &image,
                                const QSize &canvasSize,
                                const QPointF &center,
                                DrawingElement *element,
                                QString *errorMessage = nullptr);
};
