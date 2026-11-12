#pragma once

#include <QColor>
#include <QElapsedTimer>
#include <QPoint>
#include <QWidget>

class QTimer;
class QShowEvent;
class QMessageBox;
class QMenu;
class QSystemTrayIcon;
class LaserPointerMenuBuilder;
class TrailSpotWidget;

class LaserPointerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LaserPointerWidget(QWidget *parent = nullptr);
    ~LaserPointerWidget() override;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    friend class LaserPointerMenuBuilder;

    enum PointerShape {
        PointerShapeGlow = 0,
        PointerShapeRing,
        PointerShapeCross,
        PointerShapeStar,
        PointerShapeCount
    };

    enum TrailStyle {
        TrailStyleGlow = 0,
        TrailStyleDot,
        TrailStyleCount
    };

    enum Preset {
        PresetRedLaser = 0,
        PresetBlueGlow,
        PresetRainbow,
        PresetSubtle,
        PresetLargeSeminar
    };

    void chooseColor();
    void createTrailSpot(const QPoint &center);
    void decreaseSize();
    void emitTrailSpot(const QPoint &center);
    int effectivePointerSize() const;
    void decreaseBlinkInterval();
    void decreaseTrailDuration();
    void increaseSize();
    void increaseBlinkInterval();
    void increaseTrailDuration();
    void decreaseOpacity();
    void increaseOpacity();
    void resetToDefaults();
    void resetDisplaySettings();
    void resetMotionSettings();
    void resetTrailSettings();
    void applyNativeWindowHints();
    void applyPerformanceLimits();
    void ensureHelpBox();
    void rebuildStatusMenu();
    void setupStatusIcon();
    void showHelp();
    QPoint clampedTopLeft(const QPoint &topLeft) const;
    void applyPreset(int preset);
    void cyclePointerShape();
    void cycleTrailStyle();
    void loadSettings();
    void saveSettings() const;
    void setAutoFadeEnabled(bool enabled);
    void setBlinking(bool enabled);
    void setBlinkInterval(int intervalMs);
    void setClampToScreenEnabled(bool enabled);
    void setFollowCursorEnabled(bool enabled);
    void setHoldToShowEnabled(bool enabled);
    void setOpacityPercent(int opacityPercent);
    void setPointerSize(int size);
    void setPointerShape(int shape);
    void setRainbowEnabled(bool enabled);
    void setSmoothFollowEnabled(bool enabled);
    void setTrailEnabled(bool enabled);
    void setTrailDuration(int durationMs);
    void setTrailStyle(int style);
    void startBlinkAnimation();
    void startRippleAnimation();
    void stopBlinkAnimation();
    void updateBlinkOpacity();
    void updateFollowCursorPosition();
    void updateIdleOpacity();
    void updateRippleAnimation();
    void updateWindowSize();

    QColor m_color;
    QPoint m_dragOffset;
    QElapsedTimer m_blinkClock;
    QElapsedTimer m_idleClock;
    QElapsedTimer m_rippleClock;
    QPoint m_lastTrailCenter;
    QPoint m_lastCursorPos;
    QElapsedTimer m_trailClock;
    QTimer *m_blinkTimer;
    QTimer *m_followTimer;
    QTimer *m_idleTimer;
    QTimer *m_rippleTimer;
    QMessageBox *m_helpBox;
    QMenu *m_statusMenu;
    QSystemTrayIcon *m_statusIcon;
    TrailSpotWidget *m_trailLayer;
    qreal m_blinkOpacity;
    qreal m_idleOpacity;
    int m_blinkIntervalMs;
    int m_opacityPercent;
    int m_pointerSize;
    int m_pointerShape;
    int m_trailStyle;
    int m_trailDurationMs;
    bool m_autoFadeEnabled;
    bool m_blinking;
    bool m_clampToScreenEnabled;
    bool m_dragging;
    bool m_followCursorEnabled;
    bool m_hasLastTrailSpot;
    bool m_firstLaunchNotice;
    bool m_holdKeyDown;
    bool m_holdToShowEnabled;
    bool m_rainbowEnabled;
    bool m_rippleActive;
    bool m_smoothFollowEnabled;
    bool m_temporaryEnlarged;
    bool m_trailEnabled;
};
