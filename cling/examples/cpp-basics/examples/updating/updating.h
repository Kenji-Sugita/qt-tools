#ifndef UPDATING_H
#define UPDATING_H

#include <QWidget>

class Updating : public QWidget
{
    Q_OBJECT

public:
    explicit Updating(QWidget* parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

public slots:
    void updating();

private:
    const int sideLength = 50;
    const int smallRectSideLength = 8;
};
#endif
