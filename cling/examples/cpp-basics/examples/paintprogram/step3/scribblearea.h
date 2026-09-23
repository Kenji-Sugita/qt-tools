#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QColor>
class QResizeEvent;
class QPaintEvent;
class QMouseEvent;
class QPrinter;

class ScribbleArea : public QWidget
{
    Q_OBJECT

public:
    explicit ScribbleArea(QWidget* parent = nullptr);
    QSize sizeHint() const override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

public slots:
    void slotChangeColor(const QColor& color);
    void slotLoad(const QString& fileName);
    void slotSave(const QString& fileName);
    void slotPrintPreview();
    void slotPrint();
    void setPenStyle(Qt::PenStyle style);
    void setPenWidth(int width);
    void paintOnPrinter(QPrinter* printer);

private:
    QPoint lastMousePoint;
    QPixmap renderBuffer;
    QColor penColor;
    Qt::PenStyle penStyle;
    int penWidth;
};
#endif
