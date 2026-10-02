#include "LaserPointerWidget.h"

#include "LaserPointerMenuBuilder.h"
#include "LaserPointerSettings.h"
#include "TrailSpotWidget.h"

#include <QApplication>
#include <QColorDialog>
#include <QConicalGradient>
#include <QContextMenuEvent>
#include <QCursor>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLibrary>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QRadialGradient>
#include <QScreen>
#include <QShowEvent>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {
constexpr int kMinimumPointerSize = 24;
constexpr int kMaximumPointerSize = 260;
constexpr int kDefaultPointerSize = 96;
constexpr int kMinimumBlinkIntervalMs = 200;
constexpr int kMaximumBlinkIntervalMs = 1600;
constexpr int kDefaultBlinkIntervalMs = 900;
constexpr int kBlinkIntervalStepMs = 100;
constexpr int kBlinkFrameMs = 16;
constexpr int kFollowCursorFrameMs = 16;
constexpr int kIdleFrameMs = 80;
constexpr int kIdleFadeDelayMs = 1200;
constexpr int kRippleFrameMs = 16;
constexpr int kRippleDurationMs = 520;
constexpr int kTrailMinimumDistance = 10;
constexpr int kTrailMinimumIntervalMs = 45;
constexpr int kMaximumTrailSpotsPerMove = 10;
constexpr int kMinimumTrailDurationMs = 500;
constexpr int kMaximumTrailDurationMs = 6000;
constexpr int kDefaultTrailDurationMs = 2500;
constexpr int kTrailDurationStepMs = 500;
constexpr int kMinimumOpacityPercent = 25;
constexpr int kMaximumOpacityPercent = 100;
constexpr int kDefaultOpacityPercent = 100;
constexpr int kOpacityStepPercent = 10;
constexpr int kDefaultPointerShape = 0;
constexpr int kDefaultTrailStyle = 0;
constexpr qreal kTrailSizeScale = 0.56;
constexpr qreal kTrailSpacingScale = 0.48;
constexpr qreal kSmoothFollowRatio = 0.28;
constexpr qreal kPi = 3.14159265358979323846;
const QColor kDefaultColor(255, 24, 24);

#ifdef Q_OS_WIN
constexpr DWORD kDwmNcRenderingPolicy = 2;
constexpr DWORD kDwmWindowCornerPreference = 33;
constexpr DWORD kDwmBorderColor = 34;
constexpr DWORD kDwmCaptionColor = 35;
constexpr DWORD kDwmNcRenderingDisabled = 1;
constexpr DWORD kDwmCornerDoNotRound = 1;
constexpr DWORD kDwmColorNone = 0xFFFFFFFE;

using DwmSetWindowAttributeProc = HRESULT(WINAPI *)(HWND, DWORD, LPCVOID, DWORD);
#endif

QColor withAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}

void addRainbowStops(QConicalGradient &gradient, int alpha)
{
    gradient.setColorAt(0.0, withAlpha(QColor::fromHsv(0, 255, 255), alpha));
    gradient.setColorAt(1.0 / 6.0, withAlpha(QColor::fromHsv(40, 255, 255), alpha));
    gradient.setColorAt(2.0 / 6.0, withAlpha(QColor::fromHsv(75, 255, 255), alpha));
    gradient.setColorAt(3.0 / 6.0, withAlpha(QColor::fromHsv(150, 255, 255), alpha));
    gradient.setColorAt(4.0 / 6.0, withAlpha(QColor::fromHsv(210, 255, 255), alpha));
    gradient.setColorAt(5.0 / 6.0, withAlpha(QColor::fromHsv(280, 255, 255), alpha));
    gradient.setColorAt(1.0, withAlpha(QColor::fromHsv(359, 255, 255), alpha));
}

qreal smoothStep(qreal value)
{
    return value * value * (3.0 - 2.0 * value);
}

bool usesJapaneseHelp();

bool usesJapaneseHelp()
{
    return QLocale::system().language() == QLocale::Japanese;
}
} // namespace

LaserPointerWidget::LaserPointerWidget(QWidget *parent)
    : QWidget(parent)
    , m_color(kDefaultColor)
    , m_blinkTimer(new QTimer(this))
    , m_followTimer(new QTimer(this))
    , m_idleTimer(new QTimer(this))
    , m_rippleTimer(new QTimer(this))
    , m_helpBox(nullptr)
    , m_statusMenu(nullptr)
    , m_statusIcon(nullptr)
    , m_trailLayer(new TrailSpotWidget)
    , m_blinkOpacity(1.0)
    , m_idleOpacity(1.0)
    , m_blinkIntervalMs(kDefaultBlinkIntervalMs)
    , m_opacityPercent(kDefaultOpacityPercent)
    , m_pointerSize(kDefaultPointerSize)
    , m_pointerShape(kDefaultPointerShape)
    , m_trailStyle(kDefaultTrailStyle)
    , m_trailDurationMs(kDefaultTrailDurationMs)
    , m_autoFadeEnabled(false)
    , m_blinking(false)
    , m_clampToScreenEnabled(false)
    , m_dragging(false)
    , m_followCursorEnabled(false)
    , m_hasLastTrailSpot(false)
    , m_firstLaunchNotice(false)
    , m_holdKeyDown(false)
    , m_holdToShowEnabled(false)
    , m_rainbowEnabled(false)
    , m_rippleActive(false)
    , m_smoothFollowEnabled(false)
    , m_temporaryEnlarged(false)
    , m_trailEnabled(false)
{
    setWindowTitle(tr("Laser Pointer"));
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_MacAlwaysShowToolWindow);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setWindowFlags(Qt::FramelessWindowHint
                   | Qt::Tool
                   | Qt::WindowStaysOnTopHint
                   | Qt::NoDropShadowWindowHint);

    connect(m_blinkTimer, &QTimer::timeout, this, &LaserPointerWidget::updateBlinkOpacity);
    connect(m_followTimer, &QTimer::timeout, this, &LaserPointerWidget::updateFollowCursorPosition);
    connect(m_idleTimer, &QTimer::timeout, this, &LaserPointerWidget::updateIdleOpacity);
    connect(m_rippleTimer, &QTimer::timeout, this, &LaserPointerWidget::updateRippleAnimation);

    loadSettings();
    updateWindowSize();
    if (m_blinking) {
        startBlinkAnimation();
    }
    if (m_followCursorEnabled) {
        m_followTimer->start(kFollowCursorFrameMs);
    }
    if (m_autoFadeEnabled) {
        m_lastCursorPos = QCursor::pos();
        m_idleClock.start();
        m_idleTimer->start(kIdleFrameMs);
    }
    applyNativeWindowHints();
    setupStatusIcon();
    m_trailClock.start();
}

LaserPointerWidget::~LaserPointerWidget()
{
    delete m_helpBox;
    delete m_trailLayer;
}

void LaserPointerWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    LaserPointerMenuBuilder::populate(&menu, this, false);
    menu.exec(event->globalPos());
}

void LaserPointerWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QWidget::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        increaseSize();
        break;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        decreaseSize();
        break;
    case Qt::Key_B:
        setBlinking(!m_blinking);
        break;
    case Qt::Key_A:
        setAutoFadeEnabled(!m_autoFadeEnabled);
        break;
    case Qt::Key_C:
        chooseColor();
        break;
    case Qt::Key_E:
        setClampToScreenEnabled(!m_clampToScreenEnabled);
        break;
    case Qt::Key_F:
        setFollowCursorEnabled(!m_followCursorEnabled);
        break;
    case Qt::Key_G:
        setSmoothFollowEnabled(!m_smoothFollowEnabled);
        break;
    case Qt::Key_H:
        if (m_holdToShowEnabled) {
            m_holdKeyDown = true;
            update();
        } else {
            QWidget::keyPressEvent(event);
        }
        break;
    case Qt::Key_M:
        setHoldToShowEnabled(!m_holdToShowEnabled);
        break;
    case Qt::Key_O:
        increaseOpacity();
        break;
    case Qt::Key_P:
        decreaseOpacity();
        break;
    case Qt::Key_S:
        cyclePointerShape();
        break;
    case Qt::Key_V:
        setRainbowEnabled(!m_rainbowEnabled);
        break;
    case Qt::Key_T:
        setTrailEnabled(!m_trailEnabled);
        break;
    case Qt::Key_Y:
        cycleTrailStyle();
        break;
    case Qt::Key_Space: {
        const QPoint center = geometry().center();
        m_temporaryEnlarged = true;
        updateWindowSize();
        move(clampedTopLeft(center - rect().center()));
        update();
        break;
    }
    case Qt::Key_R:
        resetToDefaults();
        break;
    case Qt::Key_Comma:
    case Qt::Key_Less:
        increaseBlinkInterval();
        break;
    case Qt::Key_Period:
    case Qt::Key_Greater:
        decreaseBlinkInterval();
        break;
    case Qt::Key_BracketLeft:
        decreaseTrailDuration();
        break;
    case Qt::Key_BracketRight:
        increaseTrailDuration();
        break;
    case Qt::Key_Escape:
    case Qt::Key_Q:
        qApp->quit();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void LaserPointerWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QWidget::keyReleaseEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_H:
        if (m_holdToShowEnabled) {
            m_holdKeyDown = false;
            update();
            return;
        }
        break;
    case Qt::Key_Space:
        if (m_temporaryEnlarged) {
            const QPoint center = geometry().center();
            m_temporaryEnlarged = false;
            updateWindowSize();
            move(clampedTopLeft(center - rect().center()));
            update();
            return;
        }
        break;
    default:
        break;
    }

    QWidget::keyReleaseEvent(event);
}

void LaserPointerWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging) {
        return;
    }

    const QPoint previousCenter = geometry().center();
    const QPoint newTopLeft = clampedTopLeft(event->globalPosition().toPoint() - m_dragOffset);
    move(newTopLeft);
    emitTrailSpot(previousCenter);
}

void LaserPointerWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    startRippleAnimation();

    if (m_followCursorEnabled) {
        event->accept();
        return;
    }

    m_dragging = true;
    stopBlinkAnimation();
    m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
    m_lastTrailCenter = geometry().center();
    m_hasLastTrailSpot = true;
    m_trailClock.restart();
    event->accept();
}

void LaserPointerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        if (m_blinking) {
            startBlinkAnimation();
        }
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void LaserPointerWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    if (m_holdToShowEnabled && !m_holdKeyDown) {
        return;
    }

    painter.setOpacity(m_blinkOpacity
                       * m_idleOpacity
                       * (m_opacityPercent / static_cast<qreal>(kMaximumOpacityPercent)));

    const QPointF center(width() / 2.0, height() / 2.0);
    const qreal radius = effectivePointerSize() / 2.0;

    painter.setPen(Qt::NoPen);

    if (m_rainbowEnabled) {
        QConicalGradient rainbow(center, 0.0);
        addRainbowStops(rainbow, 225);
        painter.setBrush(rainbow);
        painter.drawEllipse(center, radius, radius);

        QRadialGradient fade(center, radius);
        fade.setColorAt(0.0, withAlpha(Qt::transparent, 0));
        fade.setColorAt(0.38, withAlpha(Qt::transparent, 0));
        fade.setColorAt(0.70, withAlpha(Qt::black, 120));
        fade.setColorAt(1.0, withAlpha(Qt::black, 255));
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        painter.setBrush(fade);
        painter.drawEllipse(center, radius, radius);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    } else {
        QRadialGradient glow(center, radius);
        glow.setColorAt(0.0, withAlpha(Qt::white, 230));
        glow.setColorAt(0.10, withAlpha(m_color.lighter(170), 245));
        glow.setColorAt(0.28, withAlpha(m_color, 220));
        glow.setColorAt(0.58, withAlpha(m_color, 90));
        glow.setColorAt(1.0, withAlpha(m_color, 0));
        painter.setBrush(glow);
        painter.drawEllipse(center, radius, radius);
    }

    const qreal coreRadius = radius * 0.18;
    QRadialGradient core(center, coreRadius);
    core.setColorAt(0.0, withAlpha(Qt::white, 255));
    core.setColorAt(0.55,
                    m_rainbowEnabled ? withAlpha(Qt::white, 230)
                                     : withAlpha(m_color.lighter(150), 245));
    core.setColorAt(1.0,
                    m_rainbowEnabled ? withAlpha(Qt::white, 130)
                                     : withAlpha(m_color.darker(120), 180));
    painter.setBrush(core);
    painter.drawEllipse(center, coreRadius, coreRadius);

    if (m_pointerShape == PointerShapeRing) {
        QPen ringPen(withAlpha(Qt::white, 220), qMax<qreal>(2.0, radius * 0.08));
        ringPen.setCapStyle(Qt::RoundCap);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(ringPen);
        painter.drawEllipse(center, radius * 0.48, radius * 0.48);
    } else if (m_pointerShape == PointerShapeCross) {
        QPen crossPen(withAlpha(Qt::white, 220), qMax<qreal>(2.0, radius * 0.07));
        crossPen.setCapStyle(Qt::RoundCap);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(crossPen);
        const qreal arm = radius * 0.48;
        painter.drawLine(QPointF(center.x() - arm, center.y()), QPointF(center.x() + arm, center.y()));
        painter.drawLine(QPointF(center.x(), center.y() - arm), QPointF(center.x(), center.y() + arm));
    } else if (m_pointerShape == PointerShapeStar) {
        QPolygonF star;
        for (int i = 0; i < 10; ++i) {
            const qreal angle = -kPi / 2.0 + i * kPi / 5.0;
            const qreal pointRadius = (i % 2 == 0) ? radius * 0.45 : radius * 0.20;
            star << QPointF(center.x() + std::cos(angle) * pointRadius,
                            center.y() + std::sin(angle) * pointRadius);
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(withAlpha(Qt::white, 190));
        painter.drawPolygon(star);
    }

    if (m_rippleActive) {
        const qreal progress = qMin(1.0,
                                    m_rippleClock.elapsed()
                                        / static_cast<qreal>(kRippleDurationMs));
        const qreal easedProgress = 1.0 - (1.0 - progress) * (1.0 - progress);
        const qreal rippleRadius = radius * (0.28 + 0.70 * easedProgress);
        const int rippleAlpha = qBound(0, static_cast<int>((1.0 - progress) * 210), 210);
        QColor rippleColor(255, 255, 255);
        rippleColor.setAlpha(rippleAlpha);

        QPen ripplePen(rippleColor, qMax<qreal>(2.0, radius * 0.055));
        ripplePen.setCapStyle(Qt::RoundCap);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(ripplePen);
        painter.drawEllipse(center, rippleRadius, rippleRadius);
    }
}

void LaserPointerWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    applyNativeWindowHints();
}

void LaserPointerWidget::wheelEvent(QWheelEvent *event)
{
    const int steps = event->angleDelta().y() / 120;
    if (steps == 0) {
        QWidget::wheelEvent(event);
        return;
    }

    setPointerSize(m_pointerSize + steps * 8);
    event->accept();
}

void LaserPointerWidget::chooseColor()
{
    if (m_rainbowEnabled) {
        return;
    }

    const QColor color = QColorDialog::getColor(m_color, this, tr("Laser Pointer Color"));
    if (!color.isValid()) {
        return;
    }

    m_color = color;
    saveSettings();
    update();
}

void LaserPointerWidget::applyNativeWindowHints()
{
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (!hwnd) {
        return;
    }

    const LONG_PTR extendedStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, extendedStyle | WS_EX_LAYERED);

    const LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    SetWindowLongPtr(hwnd,
                     GWL_STYLE,
                     style & ~(WS_CAPTION | WS_THICKFRAME | WS_BORDER | WS_DLGFRAME | WS_SYSMENU));

    SetWindowPos(hwnd,
                 nullptr,
                 0,
                 0,
                 0,
                 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

    QLibrary dwmapi(QStringLiteral("dwmapi"));
    auto setWindowAttribute = reinterpret_cast<DwmSetWindowAttributeProc>(
        dwmapi.resolve("DwmSetWindowAttribute"));
    if (!setWindowAttribute) {
        return;
    }

    const DWORD ncRenderingPolicy = kDwmNcRenderingDisabled;
    setWindowAttribute(hwnd,
                       kDwmNcRenderingPolicy,
                       &ncRenderingPolicy,
                       sizeof(ncRenderingPolicy));

    const DWORD cornerPreference = kDwmCornerDoNotRound;
    setWindowAttribute(hwnd,
                       kDwmWindowCornerPreference,
                       &cornerPreference,
                       sizeof(cornerPreference));

    const DWORD transparentColor = kDwmColorNone;
    setWindowAttribute(hwnd, kDwmBorderColor, &transparentColor, sizeof(transparentColor));
    setWindowAttribute(hwnd, kDwmCaptionColor, &transparentColor, sizeof(transparentColor));
#endif
}

void LaserPointerWidget::applyPerformanceLimits()
{
    const bool expensiveTrail = m_trailEnabled
                                && (m_pointerSize >= 140
                                    || (m_rainbowEnabled && m_smoothFollowEnabled));
    if (expensiveTrail && m_trailDurationMs > 3000) {
        m_trailDurationMs = 3000;
        saveSettings();
    }
}

void LaserPointerWidget::ensureHelpBox()
{
    if (!m_helpBox) {
        m_helpBox = new QMessageBox(nullptr);
        m_helpBox->setIcon(QMessageBox::Information);
        m_helpBox->setWindowTitle(tr("Laser Pointer Help"));
        m_helpBox->setStandardButtons(QMessageBox::Ok);
        m_helpBox->setWindowModality(Qt::NonModal);
        m_helpBox->setWindowFlags(Qt::Dialog
                                  | Qt::WindowStaysOnTopHint
                                  | Qt::WindowTitleHint
                                  | Qt::WindowCloseButtonHint);
    }

    if (usesJapaneseHelp()) {
        m_helpBox->setText(tr("マウス\n"
                              "- 左ドラッグ: ポインターを移動\n"
                              "- 左クリック: 波紋を表示\n"
                              "- マウスホイール: サイズ変更\n"
                              "- 右クリック: メニューを開く\n"
                              "- メニューバーアイコン: メニューを開く\n\n"
                              "表示\n"
                              "- + / -: 大きく / 小さく\n"
                              "- O / P: 濃く / 薄く\n"
                              "- S: 形状を切り替え\n"
                              "- Space: 押している間だけ一時拡大\n"
                              "- C: 色を選択\n"
                              "- V: 虹色\n\n"
                              "移動\n"
                              "- F: マウスポインターに追従\n"
                              "- G: 滑らかに追従\n"
                              "- E: 画面内に収める\n\n"
                              "点滅と軌跡\n"
                              "- B: 点滅\n"
                              "- , / .: 点滅を遅く / 速く\n"
                              "- T: 軌跡\n"
                              "- Y: 軌跡スタイルを切り替え\n"
                              "- [ / ]: 軌跡を短く / 長く\n\n"
                              "表示制御\n"
                              "- A: 自動フェード\n"
                              "- M: H を押している間だけ表示するモード\n"
                              "- H: Hold H to Show が有効なとき、押している間だけ表示\n\n"
                              "その他\n"
                              "- R: リセット\n"
                              "- Esc / Q: 終了"));
        return;
    }

    m_helpBox->setText(tr("Mouse\n"
                          "- Left drag: Move the pointer\n"
                          "- Left click: Show ripple\n"
                          "- Mouse wheel: Resize\n"
                          "- Right click: Open this menu\n"
                          "- Menu bar icon: Open the menu\n\n"
                          "Display\n"
                          "- + / -: Larger / Smaller\n"
                          "- O / P: More / less opaque\n"
                          "- S: Cycle shape\n"
                          "- Space: Temporarily enlarge\n"
                          "- C: Choose color\n"
                          "- V: Rainbow\n\n"
                          "Motion\n"
                          "- F: Follow cursor\n"
                          "- G: Smooth follow\n"
                          "- E: Keep on screen\n\n"
                          "Blink and Trail\n"
                          "- B: Blink\n"
                          "- , / .: Slower / faster blink\n"
                          "- T: Trail\n"
                          "- Y: Cycle trail style\n"
                          "- [ / ]: Shorter / longer trail\n\n"
                          "Visibility\n"
                          "- A: Auto fade\n"
                          "- M: Hold H to Show mode\n"
                          "- H: Show while held, when Hold H to Show is enabled\n\n"
                          "Other\n"
                          "- R: Reset\n"
                          "- Esc / Q: Quit"));
}

void LaserPointerWidget::showHelp()
{
    ensureHelpBox();
    m_helpBox->show();
    m_helpBox->raise();
    m_helpBox->activateWindow();
    for (int delayMs : {50, 200, 500}) {
        QTimer::singleShot(delayMs, m_helpBox, [box = m_helpBox]() {
            box->show();
            box->raise();
            box->activateWindow();
        });
    }
}

void LaserPointerWidget::setupStatusIcon()
{
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    m_statusMenu = new QMenu(this);
    connect(m_statusMenu, &QMenu::aboutToShow, this, &LaserPointerWidget::rebuildStatusMenu);
    rebuildStatusMenu();

    m_statusIcon = new QSystemTrayIcon(QIcon(QStringLiteral(":/assets/laser-pointer.png")), this);
    m_statusIcon->setToolTip(tr("Laser Pointer"));
    m_statusIcon->setContextMenu(m_statusMenu);
    m_statusIcon->show();
#endif
}

void LaserPointerWidget::rebuildStatusMenu()
{
    if (!m_statusMenu) {
        return;
    }

    m_statusMenu->clear();
    LaserPointerMenuBuilder::populate(m_statusMenu, this, true);
}

void LaserPointerWidget::decreaseSize()
{
    setPointerSize(m_pointerSize - 12);
}

void LaserPointerWidget::decreaseOpacity()
{
    setOpacityPercent(m_opacityPercent - kOpacityStepPercent);
}

void LaserPointerWidget::increaseOpacity()
{
    setOpacityPercent(m_opacityPercent + kOpacityStepPercent);
}

int LaserPointerWidget::effectivePointerSize() const
{
    const qreal scale = m_temporaryEnlarged ? 1.35 : 1.0;
    return std::clamp(static_cast<int>(std::lround(m_pointerSize * scale)),
                      kMinimumPointerSize,
                      static_cast<int>(std::lround(kMaximumPointerSize * 1.35)));
}

void LaserPointerWidget::createTrailSpot(const QPoint &center)
{
    const int trailSize = std::max(kMinimumPointerSize,
                                   static_cast<int>(std::lround(m_pointerSize * kTrailSizeScale)));
    m_trailLayer->addSpot(m_color,
                          m_rainbowEnabled,
                          m_trailStyle,
                          trailSize,
                          m_trailDurationMs,
                          center);
}

void LaserPointerWidget::decreaseBlinkInterval()
{
    setBlinkInterval(m_blinkIntervalMs - kBlinkIntervalStepMs);
}

void LaserPointerWidget::decreaseTrailDuration()
{
    setTrailDuration(m_trailDurationMs - kTrailDurationStepMs);
}

void LaserPointerWidget::emitTrailSpot(const QPoint &center)
{
    if (!m_trailEnabled) {
        return;
    }

    if (!m_hasLastTrailSpot) {
        createTrailSpot(center);
        m_lastTrailCenter = center;
        m_hasLastTrailSpot = true;
        m_trailClock.restart();
        return;
    }

    const QPoint delta = center - m_lastTrailCenter;
    const qreal distance = std::hypot(delta.x(), delta.y());
    if (distance < kTrailMinimumDistance && m_trailClock.elapsed() < kTrailMinimumIntervalMs) {
        return;
    }

    const qreal spacing = std::max<qreal>(kTrailMinimumDistance,
                                          m_pointerSize * kTrailSpacingScale);
    const int steps = std::clamp(static_cast<int>(std::ceil(distance / spacing)),
                                 1,
                                 kMaximumTrailSpotsPerMove);
    for (int i = 1; i <= steps; ++i) {
        const qreal t = i / static_cast<qreal>(steps);
        createTrailSpot(QPoint(qRound(m_lastTrailCenter.x() + delta.x() * t),
                               qRound(m_lastTrailCenter.y() + delta.y() * t)));
    }

    m_lastTrailCenter = center;
    m_trailClock.restart();
}

void LaserPointerWidget::increaseSize()
{
    setPointerSize(m_pointerSize + 12);
}

void LaserPointerWidget::increaseBlinkInterval()
{
    setBlinkInterval(m_blinkIntervalMs + kBlinkIntervalStepMs);
}

void LaserPointerWidget::increaseTrailDuration()
{
    setTrailDuration(m_trailDurationMs + kTrailDurationStepMs);
}

void LaserPointerWidget::resetToDefaults()
{
    m_color = kDefaultColor;
    setBlinking(false);
    setBlinkInterval(kDefaultBlinkIntervalMs);
    resetDisplaySettings();
    resetMotionSettings();
    resetTrailSettings();
    saveSettings();
    update();
}

void LaserPointerWidget::resetDisplaySettings()
{
    m_color = kDefaultColor;
    setAutoFadeEnabled(false);
    setHoldToShowEnabled(false);
    setOpacityPercent(kDefaultOpacityPercent);
    setPointerShape(kDefaultPointerShape);
    setRainbowEnabled(false);
    setPointerSize(kDefaultPointerSize);
    saveSettings();
    update();
}

void LaserPointerWidget::resetMotionSettings()
{
    setClampToScreenEnabled(false);
    setFollowCursorEnabled(false);
    setSmoothFollowEnabled(false);
    saveSettings();
}

void LaserPointerWidget::resetTrailSettings()
{
    setTrailEnabled(false);
    setTrailStyle(kDefaultTrailStyle);
    setTrailDuration(kDefaultTrailDurationMs);
    saveSettings();
}

QPoint LaserPointerWidget::clampedTopLeft(const QPoint &topLeft) const
{
    if (!m_clampToScreenEnabled) {
        return topLeft;
    }

    QScreen *screen = QGuiApplication::screenAt(topLeft + rect().center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return topLeft;
    }

    const QRect available = screen->availableGeometry();
    return QPoint(std::clamp(topLeft.x(), available.left(), available.right() - width() + 1),
                  std::clamp(topLeft.y(), available.top(), available.bottom() - height() + 1));
}

void LaserPointerWidget::applyPreset(int preset)
{
    switch (preset) {
    case PresetBlueGlow:
        m_color = QColor(42, 148, 255);
        setRainbowEnabled(false);
        setPointerShape(PointerShapeGlow);
        setOpacityPercent(90);
        setPointerSize(104);
        setTrailEnabled(true);
        setTrailStyle(TrailStyleGlow);
        break;
    case PresetRainbow:
        setRainbowEnabled(true);
        setPointerShape(PointerShapeGlow);
        setOpacityPercent(100);
        setPointerSize(108);
        setFollowCursorEnabled(true);
        setSmoothFollowEnabled(true);
        setTrailEnabled(true);
        setTrailStyle(TrailStyleDot);
        break;
    case PresetSubtle:
        m_color = QColor(255, 80, 80);
        setRainbowEnabled(false);
        setPointerShape(PointerShapeRing);
        setOpacityPercent(55);
        setPointerSize(72);
        setTrailEnabled(false);
        break;
    case PresetLargeSeminar:
        m_color = kDefaultColor;
        setRainbowEnabled(false);
        setPointerShape(PointerShapeCross);
        setOpacityPercent(100);
        setPointerSize(140);
        setTrailEnabled(true);
        setTrailStyle(TrailStyleGlow);
        setTrailDuration(3500);
        break;
    case PresetRedLaser:
    default:
        m_color = kDefaultColor;
        setRainbowEnabled(false);
        setPointerShape(PointerShapeGlow);
        setOpacityPercent(100);
        setPointerSize(kDefaultPointerSize);
        setTrailEnabled(false);
        break;
    }

    saveSettings();
    update();
}

void LaserPointerWidget::cyclePointerShape()
{
    setPointerShape((m_pointerShape + 1) % PointerShapeCount);
}

void LaserPointerWidget::cycleTrailStyle()
{
    setTrailStyle((m_trailStyle + 1) % TrailStyleCount);
}

void LaserPointerWidget::loadSettings()
{
    const LaserPointerSettings::Values values = LaserPointerSettings::load();

    m_color = values.color;
    m_autoFadeEnabled = values.autoFadeEnabled;
    m_clampToScreenEnabled = values.clampToScreenEnabled;
    m_holdToShowEnabled = values.holdToShowEnabled;
    m_opacityPercent = values.opacityPercent;
    m_pointerShape = values.pointerShape;
    m_rainbowEnabled = values.rainbowEnabled;
    m_pointerSize = values.pointerSize;
    m_followCursorEnabled = values.followCursorEnabled;
    m_smoothFollowEnabled = values.smoothFollowEnabled;
    m_blinking = values.blinking;
    m_blinkIntervalMs = values.blinkIntervalMs;
    m_trailEnabled = values.trailEnabled;
    m_trailStyle = values.trailStyle;
    m_trailDurationMs = values.trailDurationMs;
    m_firstLaunchNotice = values.firstLaunchNotice;
}

void LaserPointerWidget::saveSettings() const
{
    LaserPointerSettings::save({
        m_color,
        m_autoFadeEnabled,
        m_clampToScreenEnabled,
        m_holdToShowEnabled,
        m_opacityPercent,
        m_pointerShape,
        m_rainbowEnabled,
        m_pointerSize,
        m_followCursorEnabled,
        m_smoothFollowEnabled,
        m_blinking,
        m_blinkIntervalMs,
        m_trailEnabled,
        m_trailStyle,
        m_trailDurationMs,
        m_firstLaunchNotice,
    });
}

void LaserPointerWidget::setAutoFadeEnabled(bool enabled)
{
    if (m_autoFadeEnabled == enabled) {
        return;
    }

    m_autoFadeEnabled = enabled;
    m_idleOpacity = 1.0;
    m_lastCursorPos = QCursor::pos();
    m_idleClock.restart();
    if (m_autoFadeEnabled) {
        m_idleTimer->start(kIdleFrameMs);
    } else {
        m_idleTimer->stop();
    }
    saveSettings();
    update();
}

void LaserPointerWidget::setBlinking(bool enabled)
{
    if (m_blinking == enabled) {
        return;
    }

    m_blinking = enabled;

    if (m_blinking && !m_dragging) {
        startBlinkAnimation();
    } else {
        stopBlinkAnimation();
    }

    saveSettings();
    update();
}

void LaserPointerWidget::setBlinkInterval(int intervalMs)
{
    const int boundedInterval = std::clamp(intervalMs,
                                           kMinimumBlinkIntervalMs,
                                           kMaximumBlinkIntervalMs);
    if (m_blinkIntervalMs == boundedInterval) {
        return;
    }

    m_blinkIntervalMs = boundedInterval;
    if (m_blinking && !m_dragging) {
        startBlinkAnimation();
    }
    saveSettings();
}

void LaserPointerWidget::setClampToScreenEnabled(bool enabled)
{
    if (m_clampToScreenEnabled == enabled) {
        return;
    }

    m_clampToScreenEnabled = enabled;
    move(clampedTopLeft(frameGeometry().topLeft()));
    saveSettings();
}

void LaserPointerWidget::setFollowCursorEnabled(bool enabled)
{
    if (m_followCursorEnabled == enabled) {
        return;
    }

    m_followCursorEnabled = enabled;
    m_dragging = false;
    m_hasLastTrailSpot = false;
    if (m_followCursorEnabled) {
        updateFollowCursorPosition();
        m_followTimer->start(kFollowCursorFrameMs);
    } else {
        m_followTimer->stop();
    }
    applyPerformanceLimits();
    saveSettings();
}

void LaserPointerWidget::setHoldToShowEnabled(bool enabled)
{
    if (m_holdToShowEnabled == enabled) {
        return;
    }

    m_holdToShowEnabled = enabled;
    m_holdKeyDown = false;
    saveSettings();
    update();
}

void LaserPointerWidget::setOpacityPercent(int opacityPercent)
{
    const int boundedOpacity = std::clamp(opacityPercent,
                                          kMinimumOpacityPercent,
                                          kMaximumOpacityPercent);
    if (m_opacityPercent == boundedOpacity) {
        return;
    }

    m_opacityPercent = boundedOpacity;
    saveSettings();
    update();
}

void LaserPointerWidget::setPointerSize(int size)
{
    const int boundedSize = std::clamp(size, kMinimumPointerSize, kMaximumPointerSize);
    if (m_pointerSize == boundedSize) {
        return;
    }

    const QPoint center = geometry().center();
    m_pointerSize = boundedSize;
    updateWindowSize();
    applyNativeWindowHints();
    move(clampedTopLeft(center - rect().center()));
    applyPerformanceLimits();
    saveSettings();
}

void LaserPointerWidget::setPointerShape(int shape)
{
    const int boundedShape = std::clamp(shape, 0, PointerShapeCount - 1);
    if (m_pointerShape == boundedShape) {
        return;
    }

    m_pointerShape = boundedShape;
    saveSettings();
    update();
}

void LaserPointerWidget::setRainbowEnabled(bool enabled)
{
    if (m_rainbowEnabled == enabled) {
        return;
    }

    m_rainbowEnabled = enabled;
    applyPerformanceLimits();
    saveSettings();
    update();
}

void LaserPointerWidget::setSmoothFollowEnabled(bool enabled)
{
    if (m_smoothFollowEnabled == enabled) {
        return;
    }

    m_smoothFollowEnabled = enabled;
    applyPerformanceLimits();
    saveSettings();
}

void LaserPointerWidget::setTrailEnabled(bool enabled)
{
    if (m_trailEnabled == enabled) {
        return;
    }

    m_trailEnabled = enabled;
    m_hasLastTrailSpot = false;
    if (!m_trailEnabled) {
        m_trailLayer->clearSpots();
    }
    applyPerformanceLimits();
    saveSettings();
}

void LaserPointerWidget::setTrailStyle(int style)
{
    const int boundedStyle = std::clamp(style, 0, TrailStyleCount - 1);
    if (m_trailStyle == boundedStyle) {
        return;
    }

    m_trailStyle = boundedStyle;
    saveSettings();
}

void LaserPointerWidget::setTrailDuration(int durationMs)
{
    const bool expensiveTrail = m_trailEnabled
                                && (m_pointerSize >= 140
                                    || (m_rainbowEnabled && m_smoothFollowEnabled));
    const int maximumDuration = expensiveTrail ? 3000 : kMaximumTrailDurationMs;
    const int boundedDuration = std::clamp(durationMs, kMinimumTrailDurationMs, maximumDuration);
    if (m_trailDurationMs == boundedDuration) {
        return;
    }

    m_trailDurationMs = boundedDuration;
    saveSettings();
}

void LaserPointerWidget::startBlinkAnimation()
{
    m_blinkClock.restart();
    updateBlinkOpacity();
    m_blinkTimer->start(kBlinkFrameMs);
}

void LaserPointerWidget::startRippleAnimation()
{
    m_rippleActive = true;
    m_rippleClock.restart();
    if (!m_rippleTimer->isActive()) {
        m_rippleTimer->start(kRippleFrameMs);
    }
    update();
}

void LaserPointerWidget::stopBlinkAnimation()
{
    m_blinkTimer->stop();
    m_blinkOpacity = 1.0;
    update();
}

void LaserPointerWidget::updateBlinkOpacity()
{
    if (!m_blinking || m_dragging) {
        stopBlinkAnimation();
        return;
    }

    const qreal cycleMs = m_blinkIntervalMs * 2.0;
    const qreal cyclePosition = std::fmod(m_blinkClock.elapsed(), cycleMs) / cycleMs;
    const qreal phase = cyclePosition < 0.5 ? cyclePosition * 2.0 : (1.0 - cyclePosition) * 2.0;
    m_blinkOpacity = 1.0 - smoothStep(phase);
    update();
}

void LaserPointerWidget::updateFollowCursorPosition()
{
    if (!m_followCursorEnabled) {
        return;
    }

    const QPoint previousCenter = geometry().center();
    QPoint nextTopLeft = QCursor::pos() - rect().center();
    if (m_smoothFollowEnabled) {
        const QPoint currentTopLeft = frameGeometry().topLeft();
        const QPoint delta = nextTopLeft - currentTopLeft;
        nextTopLeft = currentTopLeft
                      + QPoint(qRound(delta.x() * kSmoothFollowRatio),
                               qRound(delta.y() * kSmoothFollowRatio));
    }
    nextTopLeft = clampedTopLeft(nextTopLeft);
    if (frameGeometry().topLeft() == nextTopLeft) {
        return;
    }

    move(nextTopLeft);
    emitTrailSpot(previousCenter);
}

void LaserPointerWidget::updateIdleOpacity()
{
    if (!m_autoFadeEnabled) {
        return;
    }

    const QPoint cursorPos = QCursor::pos();
    if (cursorPos != m_lastCursorPos) {
        m_lastCursorPos = cursorPos;
        m_idleClock.restart();
        if (m_idleOpacity != 1.0) {
            m_idleOpacity = 1.0;
            update();
        }
        return;
    }

    const qreal fadeProgress = std::clamp(
        (m_idleClock.elapsed() - kIdleFadeDelayMs) / 700.0,
        0.0,
        1.0);
    const qreal nextOpacity = 1.0 - smoothStep(fadeProgress) * 0.72;
    if (!qFuzzyCompare(m_idleOpacity, nextOpacity)) {
        m_idleOpacity = nextOpacity;
        update();
    }
}

void LaserPointerWidget::updateRippleAnimation()
{
    if (!m_rippleActive) {
        m_rippleTimer->stop();
        return;
    }

    if (m_rippleClock.elapsed() >= kRippleDurationMs) {
        m_rippleActive = false;
        m_rippleTimer->stop();
    }
    update();
}

void LaserPointerWidget::updateWindowSize()
{
    const int size = effectivePointerSize();
    resize(size, size);
    setMinimumSize(kMinimumPointerSize, kMinimumPointerSize);
    setMaximumSize(static_cast<int>(std::lround(kMaximumPointerSize * 1.35)),
                   static_cast<int>(std::lround(kMaximumPointerSize * 1.35)));
}
