#pragma once

#include <QColor>
#include <QElapsedTimer>
#include <QPoint>
#include <QVector>
#include <QWidget>

class QTimer;

class TrailSpotWidget : public QWidget
{
public:
    explicit TrailSpotWidget(QWidget *parent = nullptr);

    void addSpot(const QColor &color,
                 bool rainbowEnabled,
                 int style,
                 int size,
                 int durationMs,
                 const QPoint &center);
    void clearSpots();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Spot
    {
        QColor color;
        QPoint center;
        qint64 startedMs;
        int size;
        int style;
        int durationMs;
        bool rainbowEnabled;
    };

    void pruneExpiredSpots();
    void updateFade();
    void updateLayerGeometry();

    QElapsedTimer m_elapsed;
    QTimer *m_timer;
    QVector<Spot> m_spots;
};
