#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QColor>
class QResizeEvent;
class QPaintEvent;
class QMouseEvent;

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
    void setPenStyle(Qt::PenStyle style);
    void setPenWidth(int width);

private:
    QPoint lastMousePoint;
    QPixmap renderBuffer;
    QColor penColor;
    Qt::PenStyle penStyle;
    int penWidth;
};
#endif
