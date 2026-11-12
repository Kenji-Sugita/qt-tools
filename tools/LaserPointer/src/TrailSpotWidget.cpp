#include "TrailSpotWidget.h"

#include <QConicalGradient>
#include <QGuiApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QRadialGradient>
#include <QScreen>
#include <QTimer>

#include <algorithm>

namespace {
constexpr int kTrailFrameMs = 80;
constexpr qreal kTrailOpacity = 0.30;

QColor withScaledAlpha(QColor color, int alpha, qreal opacity)
{
    color.setAlpha(qBound(0, static_cast<int>(alpha * opacity), 255));
    return color;
}

void addRainbowStops(QConicalGradient &gradient, int alpha, qreal opacity)
{
    gradient.setColorAt(0.0, withScaledAlpha(QColor::fromHsv(0, 255, 255), alpha, opacity));
    gradient.setColorAt(1.0 / 6.0, withScaledAlpha(QColor::fromHsv(40, 255, 255), alpha, opacity));
    gradient.setColorAt(2.0 / 6.0, withScaledAlpha(QColor::fromHsv(75, 255, 255), alpha, opacity));
    gradient.setColorAt(3.0 / 6.0, withScaledAlpha(QColor::fromHsv(150, 255, 255), alpha, opacity));
    gradient.setColorAt(4.0 / 6.0, withScaledAlpha(QColor::fromHsv(210, 255, 255), alpha, opacity));
    gradient.setColorAt(5.0 / 6.0, withScaledAlpha(QColor::fromHsv(280, 255, 255), alpha, opacity));
    gradient.setColorAt(1.0, withScaledAlpha(QColor::fromHsv(359, 255, 255), alpha, opacity));
}
} // namespace

TrailSpotWidget::TrailSpotWidget(QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_MacAlwaysShowToolWindow);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowFlags(Qt::FramelessWindowHint
                   | Qt::Tool
                   | Qt::WindowStaysOnTopHint
                   | Qt::WindowDoesNotAcceptFocus
                   | Qt::WindowTransparentForInput
                   | Qt::NoDropShadowWindowHint);

    connect(m_timer, &QTimer::timeout, this, &TrailSpotWidget::updateFade);
    m_elapsed.start();
}

void TrailSpotWidget::addSpot(const QColor &color,
                              bool rainbowEnabled,
                              int style,
                              int size,
                              int durationMs,
                              const QPoint &center)
{
    pruneExpiredSpots();
    updateLayerGeometry();

    m_spots.append(Spot{color, center, m_elapsed.elapsed(), size, style, durationMs, rainbowEnabled});

    if (!isVisible()) {
        show();
    }
    if (!m_timer->isActive()) {
        m_timer->start(kTrailFrameMs);
    }

    const int radius = size / 2;
    update(QRect(center - geometry().topLeft() - QPoint(radius, radius), QSize(size, size)));
}

void TrailSpotWidget::clearSpots()
{
    m_spots.clear();
    m_timer->stop();
    hide();
    update();
}

void TrailSpotWidget::paintEvent(QPaintEvent *event)
{
    if (m_spots.isEmpty()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillRect(event->rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setPen(Qt::NoPen);

    const qint64 nowMs = m_elapsed.elapsed();
    const QPoint topLeft = geometry().topLeft();
    for (const Spot &spot : m_spots) {
        const qreal progress = qMin(1.0,
                                    (nowMs - spot.startedMs) / static_cast<qreal>(spot.durationMs));
        const qreal opacity = (1.0 - progress) * (1.0 - progress) * kTrailOpacity;
        if (opacity <= 0.0) {
            continue;
        }

        const QPointF center = spot.center - topLeft;
        const qreal radius = spot.size / 2.0;

        if (spot.style == 1) {
            QColor dotColor = spot.rainbowEnabled ? QColor(255, 255, 255) : spot.color.lighter(150);
            dotColor.setAlpha(qBound(0, static_cast<int>(180 * opacity), 180));
            painter.setBrush(dotColor);
            painter.drawEllipse(center, radius * 0.42, radius * 0.42);
            continue;
        }

        if (spot.rainbowEnabled) {
            QConicalGradient rainbow(center, 0.0);
            addRainbowStops(rainbow, 185, opacity);
            painter.setBrush(rainbow);
            painter.drawEllipse(center, radius, radius);

            QRadialGradient fade(center, radius);
            fade.setColorAt(0.0, withScaledAlpha(Qt::transparent, 0, opacity));
            fade.setColorAt(0.38, withScaledAlpha(Qt::transparent, 0, opacity));
            fade.setColorAt(0.70, withScaledAlpha(Qt::black, 120, opacity));
            fade.setColorAt(1.0, withScaledAlpha(Qt::black, 255, opacity));
            painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            painter.setBrush(fade);
            painter.drawEllipse(center, radius, radius);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            continue;
        }

        QRadialGradient glow(center, radius);
        glow.setColorAt(0.0, withScaledAlpha(Qt::white, 190, opacity));
        glow.setColorAt(0.16, withScaledAlpha(spot.color.lighter(165), 220, opacity));
        glow.setColorAt(0.42, withScaledAlpha(spot.color, 180, opacity));
        glow.setColorAt(0.76, withScaledAlpha(spot.color, 60, opacity));
        glow.setColorAt(1.0, withScaledAlpha(spot.color, 0, opacity));

        painter.setBrush(glow);
        painter.drawEllipse(center, radius, radius);
    }
}

void TrailSpotWidget::pruneExpiredSpots()
{
    const qint64 nowMs = m_elapsed.elapsed();
    m_spots.erase(std::remove_if(m_spots.begin(),
                                 m_spots.end(),
                                 [nowMs](const Spot &spot) {
                                     return nowMs - spot.startedMs >= spot.durationMs;
                                 }),
                  m_spots.end());
}

void TrailSpotWidget::updateFade()
{
    pruneExpiredSpots();
    if (m_spots.isEmpty()) {
        m_timer->stop();
        hide();
        return;
    }

    update();
}

void TrailSpotWidget::updateLayerGeometry()
{
    QRect desktopGeometry;
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        desktopGeometry = desktopGeometry.isNull()
                              ? screen->geometry()
                              : desktopGeometry.united(screen->geometry());
    }

    if (!desktopGeometry.isNull() && geometry() != desktopGeometry) {
        setGeometry(desktopGeometry);
    }
}
