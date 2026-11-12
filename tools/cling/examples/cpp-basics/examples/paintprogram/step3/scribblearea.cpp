#include "scribblearea.h"
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QPainter>
#include <QImage>
#include <QDebug>

ScribbleArea::ScribbleArea(QWidget* parent)
    : QWidget(parent),
      penStyle(Qt::SolidLine),
      penWidth(3)
{
}

QSize ScribbleArea::sizeHint() const
{
    return QSize(400, 400);
}

void ScribbleArea::mousePressEvent(QMouseEvent* event)
{
    lastMousePoint = event->pos();
}

void ScribbleArea::mouseMoveEvent(QMouseEvent* event)
{
    QPainter painter(&renderBuffer);
    QPen pen(penColor, penWidth);
    pen.setStyle(penStyle);
    painter.setPen(pen);
    painter.drawLine(lastMousePoint, event->pos());
    const QRect updatingRect = QRect(lastMousePoint, event->pos()).normalized().adjusted(-penWidth, -penWidth, penWidth, penWidth);
    update(updatingRect);

    lastMousePoint = event->pos();
}

void ScribbleArea::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(0, 0, renderBuffer);
}

void ScribbleArea::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    if (width() > renderBuffer.width() || height() > renderBuffer.height()) {
        QPixmap newBuffer(size());
        newBuffer.fill(Qt::white);

        QPainter painter(&newBuffer);
        painter.drawPixmap(0, 0, renderBuffer);
        renderBuffer = newBuffer;
    }
}

void ScribbleArea::slotChangeColor(const QColor& color)
{
    penColor = color;
}

void ScribbleArea::slotLoad(const QString& filename)
{
    QImage image;
    if (!image.load(filename)) {
        QMessageBox::warning(this, "Load error", "Could not load file");
	return;
    }
    renderBuffer = QPixmap(image.size());
    renderBuffer.fill(Qt::white);
    QPainter painter(&renderBuffer);
    painter.drawImage(0, 0, image);
    painter.end();

    resize(image.size());

    update();
}

void ScribbleArea::slotSave(const QString& filename)
{
    if (!renderBuffer.save(filename)) {
        QMessageBox::warning(this, "Save error", "Could not save file");
    }
}

void ScribbleArea::slotPrintPreview()
{
    QPrintPreviewDialog dialog(this);
    connect(&dialog, &QPrintPreviewDialog::paintRequested, this, &ScribbleArea::paintOnPrinter);
    dialog.exec();
}

void ScribbleArea::slotPrint()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (!dialog.exec()) {
        return;
    }
    paintOnPrinter(&printer);
}

void ScribbleArea::paintOnPrinter(QPrinter* printer)
{
    const QRect pageRect = printer->pageLayout().paintRectPixels(printer->resolution());
    const int paperWidth = pageRect.width();
    const int paperHeight = pageRect.height();

    QPainter painter(printer);
    int pageNumber = 0;
    const int margin = 3;
    for (int y = 0; y < height(); y += paperHeight - margin) {
        for (int x = 0; x < width(); x += paperWidth - margin) {
            pageNumber += 1;
            if (pageNumber > 1) {
                printer->newPage();
            }

            painter.drawPixmap(QPoint(1, 1), renderBuffer, QRect(x, y, paperWidth - margin, paperHeight - margin));
            painter.drawText(paperWidth - 50, paperHeight - 20, QString("Page %1").arg(pageNumber));
            painter.drawRect(0, 0, paperWidth - 1, paperHeight - 1);
        }
    }
}

void ScribbleArea::setPenStyle(Qt::PenStyle style)
{
    penStyle = style;
}

void ScribbleArea::setPenWidth(int width)
{
    penWidth = width;
}
