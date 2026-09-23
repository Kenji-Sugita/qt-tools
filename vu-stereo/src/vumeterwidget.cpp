// VuMeterWidget の実装です。背景 SVG の座標系に合わせて針の支点と目盛り位置を
// 定義し、dBFS から VU 目盛りへの変換、針の描画、ピークホールドの減衰処理を
// まとめています。1 つのメーター描画に必要な処理だけをここへ閉じ込めます。

#include "vumeterwidget.h"

#include <QPainter>
#include <QScreen>

#include <algorithm>
#include <array>
#include <cmath>

namespace {

constexpr QSizeF kSvgSize(1506.0, 936.0);
constexpr double kTargetWidthMm = 100.0;
constexpr double kMinDbfs = -60.0;
constexpr double kMaxDbfs = 0.0;
constexpr QPointF kNormalPivot(753.0, 747.0);
constexpr QPointF kLoweredPivot(756.0, 1215.0);
constexpr QRectF kYellowDialFace(88.0, 103.0, 1330.0, 718.0);
constexpr double kLinearScaleTranslateY = -56.6928;
constexpr double kLinearScaleY = 396.0 + kLinearScaleTranslateY;

struct TickPoint {
    double vu;
    QPointF point;
};

constexpr std::array<TickPoint, 10> kScalePoints{{
    {-20.0, {263.0, 463.0}},
    {-10.0, {397.0, 410.0}},
    {-7.0, {517.0, 377.0}},
    {-5.0, {637.0, 357.0}},
    {-3.0, {775.0, 351.0}},
    {-1.0, {919.0, 364.0}},
    {0.0, {1010.0, 382.0}},
    {1.0, {1101.0, 407.0}},
    {2.0, {1210.0, 447.0}},
    {3.0, {1305.0, 486.0}},
}};

constexpr std::array<TickPoint, 10> kLinearScalePoints{{
    {-20.0, {210.0, kLinearScaleY}},
    {-10.0, {400.0, kLinearScaleY}},
    {-7.0, {520.0, kLinearScaleY}},
    {-5.0, {640.0, kLinearScaleY}},
    {-3.0, {760.0, kLinearScaleY}},
    {-1.0, {900.0, kLinearScaleY}},
    {0.0, {1010.0, kLinearScaleY}},
    {1.0, {1110.0, kLinearScaleY}},
    {2.0, {1215.0, kLinearScaleY}},
    {3.0, {1320.0, kLinearScaleY}},
}};

double clamp(double value, double minValue, double maxValue)
{
    return std::max(minValue, std::min(value, maxValue));
}

double smoothStep(double t)
{
    t = clamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

QPointF interpolateScalePoint(const std::array<TickPoint, 10> &scalePoints, double vu)
{
    if (vu <= scalePoints.front().vu) {
        return scalePoints.front().point;
    }
    if (vu >= scalePoints.back().vu) {
        return scalePoints.back().point;
    }

    for (std::size_t i = 1; i < scalePoints.size(); ++i) {
        const TickPoint &right = scalePoints[i];
        if (vu <= right.vu) {
            const TickPoint &left = scalePoints[i - 1];
            const double t = (vu - left.vu) / (right.vu - left.vu);
            return left.point + (right.point - left.point) * smoothStep(t);
        }
    }

    return scalePoints.back().point;
}

QPointF interpolateScalePoint(double vu)
{
    return interpolateScalePoint(kScalePoints, vu);
}

QPointF interpolateLinearScalePoint(double vu)
{
    return interpolateScalePoint(kLinearScalePoints, vu);
}

int targetWidthForScreen(const QScreen *screen)
{
    constexpr int kFallbackLogicalWidth = 429;

    if (!screen) {
        return kFallbackLogicalWidth;
    }

    const QSizeF physicalSize = screen->physicalSize();
    if (physicalSize.width() > 0.0) {
        const double logicalPixelsPerMm = screen->size().width() / physicalSize.width();
        return static_cast<int>(std::round(logicalPixelsPerMm * kTargetWidthMm));
    }

    const double dpi = screen->physicalDotsPerInchX();
    const double dpr = screen->devicePixelRatio();
    if (dpi > 0.0 && dpr > 0.0) {
        return static_cast<int>(std::round((dpi / dpr) * (kTargetWidthMm / 25.4)));
    }

    return kFallbackLogicalWidth;
}

double dbfsToVu(double dbfs)
{
    const double clippedDbfs = clamp(dbfs, kMinDbfs, kMaxDbfs);

    if (clippedDbfs <= -18.0) {
        const double t = (clippedDbfs - kMinDbfs) / (-18.0 - kMinDbfs);
        return -20.0 + t * 20.0;
    }

    const double t = (clippedDbfs - (-18.0)) / (-9.0 - (-18.0));
    return clamp(t * 3.0, 0.0, 3.0);
}

} // namespace

VuMeterWidget::VuMeterWidget(QWidget *parent)
    : QWidget(parent),
      m_dbfs(kMinDbfs),
      m_peakDbfs(kMinDbfs),
      m_needleWidth(6.0),
      m_peakHoldMillisecondsRemaining(0),
      m_peakHoldEnabled(false),
      m_pivotVisible(true),
      m_needlePivotLowered(true),
      m_scaleStyle(MeterScaleStyle::Arc)
{
    setWindowTitle(tr("VuStereo"));
    setAttribute(Qt::WA_OpaquePaintEvent);
    loadBackground();

    const int targetWidth = targetWidthForScreen(screen());
    const int targetHeight = static_cast<int>(std::round(targetWidth * kSvgSize.height() / kSvgSize.width()));
    setFixedSize(targetWidth, targetHeight);
}

double VuMeterWidget::value() const
{
    return m_dbfs;
}

void VuMeterWidget::setValue(double dbfs)
{
    const double newDbfs = clamp(dbfs, kMinDbfs, kMaxDbfs);
    updatePeakHold(newDbfs);
    if (m_dbfs == newDbfs) {
        return;
    }

    m_dbfs = newDbfs;
    update();
}

void VuMeterWidget::setPeakHoldEnabled(bool enabled)
{
    if (m_peakHoldEnabled == enabled) {
        return;
    }

    m_peakHoldEnabled = enabled;
    m_peakDbfs = m_dbfs;
    m_peakHoldElapsed.restart();
    m_peakHoldMillisecondsRemaining = 0;
    update();
}

void VuMeterWidget::setPivotVisible(bool visible)
{
    if (m_pivotVisible == visible) {
        return;
    }

    m_pivotVisible = visible;
    update();
}

void VuMeterWidget::setNeedlePivotLowered(bool lowered)
{
    if (m_needlePivotLowered == lowered) {
        return;
    }

    m_needlePivotLowered = lowered;
    update();
}

void VuMeterWidget::setNeedleWidth(int width)
{
    const double newWidth = clamp(width, 1, 30);
    if (m_needleWidth == newWidth) {
        return;
    }

    m_needleWidth = newWidth;
    update();
}

void VuMeterWidget::setScaleStyle(MeterScaleStyle scaleStyle)
{
    if (m_scaleStyle == scaleStyle) {
        return;
    }

    m_scaleStyle = scaleStyle;
    loadBackground();
    update();
}

void VuMeterWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.fillRect(rect(), Qt::black);
    painter.scale(width() / kSvgSize.width(), height() / kSvgSize.height());

    m_background.render(&painter, QRectF(QPointF(0.0, 0.0), kSvgSize));
    const QPointF pivot = needlePivot();
    if (m_peakHoldEnabled) {
        drawNeedle(&painter,
                   dbfsToVu(m_peakDbfs),
                   QColor(65, 16, 10),
                   QColor(181, 21, 15),
                   QColor(255, 196, 160, 170));
    }
    drawNeedle(&painter,
               dbfsToVu(m_dbfs),
               QColor(8, 8, 7),
               QColor(20, 20, 18),
               QColor(120, 120, 105, 150));
    if (m_scaleStyle == MeterScaleStyle::Arc) {
        drawPivot(&painter, pivot);
    }
}

void VuMeterWidget::updatePeakHold(double dbfs)
{
    if (!m_peakHoldEnabled) {
        return;
    }

    constexpr qint64 kHoldMilliseconds = 1200;
    constexpr double kDecayDbPerSecond = 18.0;

    if (!m_peakHoldElapsed.isValid()) {
        m_peakHoldElapsed.start();
        m_peakDbfs = dbfs;
        return;
    }

    const qint64 elapsedMilliseconds = m_peakHoldElapsed.restart();
    if (dbfs >= m_peakDbfs) {
        m_peakDbfs = dbfs;
        m_peakHoldMillisecondsRemaining = kHoldMilliseconds;
        return;
    }

    if (m_peakHoldMillisecondsRemaining > 0) {
        m_peakHoldMillisecondsRemaining = std::max<qint64>(0, m_peakHoldMillisecondsRemaining - elapsedMilliseconds);
        return;
    }

    const double decay = kDecayDbPerSecond * (static_cast<double>(elapsedMilliseconds) / 1000.0);
    m_peakDbfs = std::max(dbfs, m_peakDbfs - decay);
}

QPointF VuMeterWidget::needlePivot() const
{
    return m_needlePivotLowered ? kLoweredPivot : kNormalPivot;
}

void VuMeterWidget::loadBackground()
{
    const QString path = m_scaleStyle == MeterScaleStyle::Linear
                             ? QStringLiteral(":/images/vu-linear-background.svg")
                             : QStringLiteral(":/images/vu-background.svg");
    m_background.load(path);
}

void VuMeterWidget::drawNeedle(QPainter *painter,
                               double vu,
                               const QColor &outlineColor,
                               const QColor &needleColor,
                               const QColor &highlightColor)
{
    if (m_scaleStyle == MeterScaleStyle::Linear) {
        drawLinearNeedle(painter, vu, outlineColor, needleColor, highlightColor);
        return;
    }

    drawArcNeedle(painter, vu, outlineColor, needleColor, highlightColor);
}

void VuMeterWidget::drawArcNeedle(QPainter *painter,
                                  double vu,
                                  const QColor &outlineColor,
                                  const QColor &needleColor,
                                  const QColor &highlightColor)
{
    const QPointF pivot = needlePivot();
    const QPointF tip = interpolateScalePoint(vu);
    const QPointF direction = tip - pivot;
    const double length = std::hypot(direction.x(), direction.y());
    if (length <= 0.0) {
        return;
    }

    const QPointF unit(direction.x() / length, direction.y() / length);
    const QPointF normal(-unit.y(), unit.x());
    const QPointF start = pivot;
    const QPointF end = tip;

    painter->save();
    painter->setClipRect(kYellowDialFace);

    painter->save();
    painter->translate(9.0, 10.0);
    painter->setPen(QPen(QColor(20, 20, 18, 70), m_needleWidth, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(start, end);
    painter->restore();

    painter->setPen(QPen(outlineColor, m_needleWidth + 2.0, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(start, end);

    painter->setPen(QPen(needleColor, m_needleWidth, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(start, end);

    painter->setPen(QPen(highlightColor, std::max(1.0, m_needleWidth * 0.18), Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(start + normal * m_needleWidth * 0.22, end + normal * m_needleWidth * 0.22);
    painter->restore();
}

void VuMeterWidget::drawLinearNeedle(QPainter *painter,
                                     double vu,
                                     const QColor &outlineColor,
                                     const QColor &needleColor,
                                     const QColor &highlightColor)
{
    const QPointF scalePoint = interpolateLinearScalePoint(vu);
    const QPointF start(scalePoint.x(), kYellowDialFace.bottom());
    const QPointF end(scalePoint.x(), kLinearScaleY);

    painter->save();
    painter->setClipRect(kYellowDialFace);

    painter->save();
    painter->translate(7.0, 8.0);
    painter->setPen(QPen(QColor(20, 20, 18, 70), m_needleWidth, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(start, end);
    painter->restore();

    painter->setPen(QPen(outlineColor, m_needleWidth + 2.0, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(start, end);

    painter->setPen(QPen(needleColor, m_needleWidth, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(start, end);

    const double highlightOffset = std::max(1.0, m_needleWidth * 0.22);
    painter->setPen(QPen(highlightColor, std::max(1.0, m_needleWidth * 0.18), Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(start + QPointF(highlightOffset, 0.0), end + QPointF(highlightOffset, 0.0));
    painter->restore();
}

void VuMeterWidget::drawPivot(QPainter *painter, const QPointF &pivot)
{
    if (!m_pivotVisible) {
        return;
    }

    painter->setPen(QPen(QColor(18, 15, 10), 4.0));
    painter->setBrush(QColor(46, 43, 34));
    painter->drawEllipse(pivot, 35.0, 35.0);

    painter->setPen(QPen(QColor(120, 105, 74), 3.0));
    painter->setBrush(QColor(26, 24, 20));
    painter->drawEllipse(pivot, 21.0, 21.0);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(255, 255, 220, 90));
    painter->drawEllipse(pivot + QPointF(-8.0, -9.0), 5.0, 4.0);
}
