#ifndef SLIDER_H
#define SLIDER_H

#include <QWidget>

class QLabel;
class QSlider;

class Slider :public QWidget
{
    Q_OBJECT

public:
    explicit Slider(Qt::Orientation orientation = Qt::Horizontal, QWidget* parent = nullptr);

    int value() const;
    int minimum() const;
    int maximum() const;
    void setMinimum(int min);
    void setMaximum(int max);

public slots:
    void setValue(int value);
    void setRange(int min, int max);

signals:
    void valueChanged(int value);

protected:
    int maxWidth(int min, int max) const;

private:
    QSlider* slider;
    QLabel* display;
};
#endif
