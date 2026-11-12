#ifndef GIZMO_H
#define GIZMO_H

#include <QWidget>
#include <QColor>
class QPaintEvent;

class Gizmo : public QWidget
{
    Q_OBJECT

public:
    Gizmo(const QColor& color1, const QColor& color2, Qt::Orientation orientation, QWidget* parent = nullptr);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QColor color1, color2;
    Qt::Orientation orientation;
};
#endif
