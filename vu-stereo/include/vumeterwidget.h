// VuMeterWidget は、1 個のアナログ VU メーターを描画するウィジェットです。
// SVG の背景に対して針、支点、ピークホールド針を重ねて描き、外部からは
// setValue() と value() を中心に操作できます。左右ステレオ表示では、このクラスを
// 2 つ並べて使います。

#pragma once

#include "metersettings.h"

#include <QColor>
#include <QElapsedTimer>
#include <QPointF>
#include <QSvgRenderer>
#include <QWidget>

class QPainter;

class VuMeterWidget final : public QWidget {
    Q_OBJECT

public:
    explicit VuMeterWidget(QWidget *parent = nullptr);

    double value() const;

public slots:
    void setValue(double dbfs);
    void setPeakHoldEnabled(bool enabled);
    void setPivotVisible(bool visible);
    void setNeedlePivotLowered(bool lowered);
    void setNeedleWidth(int width);
    void setScaleStyle(MeterScaleStyle scaleStyle);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void updatePeakHold(double dbfs);
    QPointF needlePivot() const;
    void loadBackground();
    void drawNeedle(QPainter *painter,
                    double vu,
                    const QColor &outlineColor,
                    const QColor &needleColor,
                    const QColor &highlightColor);
    void drawArcNeedle(QPainter *painter,
                       double vu,
                       const QColor &outlineColor,
                       const QColor &needleColor,
                       const QColor &highlightColor);
    void drawLinearNeedle(QPainter *painter,
                          double vu,
                          const QColor &outlineColor,
                          const QColor &needleColor,
                          const QColor &highlightColor);
    void drawPivot(QPainter *painter, const QPointF &pivot);

    QSvgRenderer m_background;
    QElapsedTimer m_peakHoldElapsed;
    double m_dbfs;
    double m_peakDbfs;
    double m_needleWidth;
    qint64 m_peakHoldMillisecondsRemaining;
    bool m_peakHoldEnabled;
    bool m_pivotVisible;
    bool m_needlePivotLowered;
    MeterScaleStyle m_scaleStyle;
};
