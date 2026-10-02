#include "LaserPointerMenuBuilder.h"

#include "LaserPointerWidget.h"

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QLocale>
#include <QMenu>
#include <QWidgetAction>

namespace {
bool usesJapaneseHelp()
{
    return QLocale::system().language() == QLocale::Japanese;
}

QString uiText(const char *english, const char *japanese)
{
    return usesJapaneseHelp() ? QString::fromUtf8(japanese) : QObject::tr(english);
}

QString onOffText(bool enabled)
{
    return enabled ? uiText("On", "オン") : uiText("Off", "オフ");
}
} // namespace

QString LaserPointerMenuBuilder::pointerShapeName(int shape)
{
    const bool japanese = usesJapaneseHelp();
    switch (shape) {
    case LaserPointerWidget::PointerShapeRing:
        return japanese ? QStringLiteral("リング") : QObject::tr("Ring");
    case LaserPointerWidget::PointerShapeCross:
        return japanese ? QStringLiteral("十字") : QObject::tr("Cross");
    case LaserPointerWidget::PointerShapeStar:
        return japanese ? QStringLiteral("星") : QObject::tr("Star");
    default:
        return japanese ? QStringLiteral("グロー") : QObject::tr("Glow");
    }
}

QString LaserPointerMenuBuilder::trailStyleName(int style)
{
    const bool japanese = usesJapaneseHelp();
    switch (style) {
    case LaserPointerWidget::TrailStyleDot:
        return japanese ? QStringLiteral("点") : QObject::tr("Dots");
    default:
        return japanese ? QStringLiteral("グロー") : QObject::tr("Glow");
    }
}

void LaserPointerMenuBuilder::populate(QMenu *menu,
                                       LaserPointerWidget *widget,
                                       bool includePointerVisibilityAction)
{
    const QString colorSummary = widget->m_rainbowEnabled ? uiText("Rainbow", "虹色")
                                                          : widget->m_color.name(QColor::HexRgb);
    menu->addAction(uiText("Status: %1 / %2 / %3% / Trail %4 / Follow %5",
                           "状態: %1 / %2 / %3% / 軌跡 %4 / 追従 %5")
                        .arg(colorSummary,
                             pointerShapeName(widget->m_pointerShape),
                             QString::number(widget->m_opacityPercent),
                             onOffText(widget->m_trailEnabled),
                             onOffText(widget->m_followCursorEnabled)));
    if (widget->m_firstLaunchNotice) {
        menu->addAction(uiText("Tip: Start with Preset > Standard or Large Seminar",
                               "ヒント: プリセット > 標準 または セミナー大画面 から始められます"));
    }
    if (includePointerVisibilityAction) {
        QAction *pointerVisibilityAction = menu->addAction(widget->isVisible()
                                                               ? uiText("Hide Pointer", "ポインターを非表示")
                                                               : uiText("Show Pointer", "ポインターを表示"));
        QObject::connect(pointerVisibilityAction, &QAction::triggered, widget, [widget]() {
            widget->setVisible(!widget->isVisible());
        });
    }
    menu->addSeparator();

    QMenu *displayMenu = menu->addMenu(uiText("Display", "表示"));
    QMenu *motionMenu = menu->addMenu(uiText("Motion", "移動"));
    QMenu *trailMenu = menu->addMenu(uiText("Trail", "軌跡"));

    QAction *blinkAction = displayMenu->addAction(uiText("Blink", "点滅"));
    blinkAction->setCheckable(true);
    blinkAction->setChecked(widget->m_blinking);
    QMenu *blinkIntervalMenu = displayMenu->addMenu(uiText("Blink Interval", "点滅間隔"));
    QAction *blinkInterval200Action = blinkIntervalMenu->addAction(QStringLiteral("0.2 s"));
    QAction *blinkInterval500Action = blinkIntervalMenu->addAction(QStringLiteral("0.5 s"));
    QAction *blinkInterval900Action = blinkIntervalMenu->addAction(QStringLiteral("0.9 s"));
    QAction *blinkInterval1200Action = blinkIntervalMenu->addAction(QStringLiteral("1.2 s"));
    QAction *blinkInterval1600Action = blinkIntervalMenu->addAction(QStringLiteral("1.6 s"));
    for (QAction *action : {blinkInterval200Action,
                            blinkInterval500Action,
                            blinkInterval900Action,
                            blinkInterval1200Action,
                            blinkInterval1600Action}) {
        action->setCheckable(true);
    }
    blinkInterval200Action->setChecked(widget->m_blinkIntervalMs == 200);
    blinkInterval500Action->setChecked(widget->m_blinkIntervalMs == 500);
    blinkInterval900Action->setChecked(widget->m_blinkIntervalMs == 900);
    blinkInterval1200Action->setChecked(widget->m_blinkIntervalMs == 1200);
    blinkInterval1600Action->setChecked(widget->m_blinkIntervalMs == 1600);

    displayMenu->addSeparator();
    QAction *largerAction = displayMenu->addAction(uiText("Larger", "大きく"));
    QAction *smallerAction = displayMenu->addAction(uiText("Smaller", "小さく"));
    QMenu *opacityMenu = displayMenu->addMenu(uiText("Opacity", "透明度"));
    QAction *opacity25Action = opacityMenu->addAction(QStringLiteral("25%"));
    QAction *opacity50Action = opacityMenu->addAction(QStringLiteral("50%"));
    QAction *opacity75Action = opacityMenu->addAction(QStringLiteral("75%"));
    QAction *opacity100Action = opacityMenu->addAction(QStringLiteral("100%"));
    for (QAction *action : {opacity25Action, opacity50Action, opacity75Action, opacity100Action}) {
        action->setCheckable(true);
    }
    opacity25Action->setChecked(widget->m_opacityPercent == 25);
    opacity50Action->setChecked(widget->m_opacityPercent == 50);
    opacity75Action->setChecked(widget->m_opacityPercent == 75);
    opacity100Action->setChecked(widget->m_opacityPercent == 100);
    QMenu *shapeMenu = displayMenu->addMenu(uiText("Shape", "形状"));
    QAction *shapeGlowAction = shapeMenu->addAction(pointerShapeName(LaserPointerWidget::PointerShapeGlow));
    QAction *shapeRingAction = shapeMenu->addAction(pointerShapeName(LaserPointerWidget::PointerShapeRing));
    QAction *shapeCrossAction = shapeMenu->addAction(pointerShapeName(LaserPointerWidget::PointerShapeCross));
    QAction *shapeStarAction = shapeMenu->addAction(pointerShapeName(LaserPointerWidget::PointerShapeStar));
    for (QAction *action : {shapeGlowAction, shapeRingAction, shapeCrossAction, shapeStarAction}) {
        action->setCheckable(true);
    }
    shapeGlowAction->setChecked(widget->m_pointerShape == LaserPointerWidget::PointerShapeGlow);
    shapeRingAction->setChecked(widget->m_pointerShape == LaserPointerWidget::PointerShapeRing);
    shapeCrossAction->setChecked(widget->m_pointerShape == LaserPointerWidget::PointerShapeCross);
    shapeStarAction->setChecked(widget->m_pointerShape == LaserPointerWidget::PointerShapeStar);
    QAction *rainbowAction = displayMenu->addAction(uiText("Rainbow", "虹色"));
    rainbowAction->setCheckable(true);
    rainbowAction->setChecked(widget->m_rainbowEnabled);
    QAction *colorAction = displayMenu->addAction(uiText("Color...", "色..."));
    colorAction->setEnabled(!widget->m_rainbowEnabled);
    displayMenu->addSeparator();
    QAction *autoFadeAction = displayMenu->addAction(uiText("Auto Fade", "自動フェード"));
    autoFadeAction->setCheckable(true);
    autoFadeAction->setChecked(widget->m_autoFadeEnabled);
    QAction *holdToShowAction = displayMenu->addAction(uiText("Hold H to Show", "H を押している間だけ表示"));
    holdToShowAction->setCheckable(true);
    holdToShowAction->setChecked(widget->m_holdToShowEnabled);

    QAction *followCursorAction = motionMenu->addAction(uiText("Follow Cursor", "マウスポインターに追従"));
    followCursorAction->setCheckable(true);
    followCursorAction->setChecked(widget->m_followCursorEnabled);
    QAction *smoothFollowAction = motionMenu->addAction(uiText("Smooth Follow", "滑らかに追従"));
    smoothFollowAction->setCheckable(true);
    smoothFollowAction->setChecked(widget->m_smoothFollowEnabled);
    smoothFollowAction->setEnabled(widget->m_followCursorEnabled);
    QAction *clampAction = motionMenu->addAction(uiText("Keep On Screen", "画面内に収める"));
    clampAction->setCheckable(true);
    clampAction->setChecked(widget->m_clampToScreenEnabled);

    QAction *trailAction = trailMenu->addAction(uiText("Trail", "軌跡"));
    trailAction->setCheckable(true);
    trailAction->setChecked(widget->m_trailEnabled);
    QMenu *trailStyleMenu = trailMenu->addMenu(uiText("Trail Style", "軌跡スタイル"));
    trailStyleMenu->setEnabled(widget->m_trailEnabled);
    QAction *trailStyleGlowAction = trailStyleMenu->addAction(trailStyleName(LaserPointerWidget::TrailStyleGlow));
    QAction *trailStyleDotAction = trailStyleMenu->addAction(trailStyleName(LaserPointerWidget::TrailStyleDot));
    trailStyleGlowAction->setCheckable(true);
    trailStyleDotAction->setCheckable(true);
    trailStyleGlowAction->setChecked(widget->m_trailStyle == LaserPointerWidget::TrailStyleGlow);
    trailStyleDotAction->setChecked(widget->m_trailStyle == LaserPointerWidget::TrailStyleDot);
    QMenu *trailDurationMenu = trailMenu->addMenu(uiText("Trail Duration", "軌跡表示時間"));
    trailDurationMenu->setEnabled(widget->m_trailEnabled);
    QAction *trailDuration500Action = trailDurationMenu->addAction(QStringLiteral("0.5 s"));
    QAction *trailDuration1000Action = trailDurationMenu->addAction(QStringLiteral("1.0 s"));
    QAction *trailDuration1500Action = trailDurationMenu->addAction(QStringLiteral("1.5 s"));
    QAction *trailDuration2000Action = trailDurationMenu->addAction(QStringLiteral("2.0 s"));
    QAction *trailDuration2500Action = trailDurationMenu->addAction(QStringLiteral("2.5 s"));
    QAction *trailDuration3000Action = trailDurationMenu->addAction(QStringLiteral("3.0 s"));
    QAction *trailDuration4000Action = trailDurationMenu->addAction(QStringLiteral("4.0 s"));
    QAction *trailDuration5000Action = trailDurationMenu->addAction(QStringLiteral("5.0 s"));
    QAction *trailDuration6000Action = trailDurationMenu->addAction(QStringLiteral("6.0 s"));
    for (QAction *action : {trailDuration500Action,
                            trailDuration1000Action,
                            trailDuration1500Action,
                            trailDuration2000Action,
                            trailDuration2500Action,
                            trailDuration3000Action,
                            trailDuration4000Action,
                            trailDuration5000Action,
                            trailDuration6000Action}) {
        action->setCheckable(true);
    }
    trailDuration500Action->setChecked(widget->m_trailDurationMs == 500);
    trailDuration1000Action->setChecked(widget->m_trailDurationMs == 1000);
    trailDuration1500Action->setChecked(widget->m_trailDurationMs == 1500);
    trailDuration2000Action->setChecked(widget->m_trailDurationMs == 2000);
    trailDuration2500Action->setChecked(widget->m_trailDurationMs == 2500);
    trailDuration3000Action->setChecked(widget->m_trailDurationMs == 3000);
    trailDuration4000Action->setChecked(widget->m_trailDurationMs == 4000);
    trailDuration5000Action->setChecked(widget->m_trailDurationMs == 5000);
    trailDuration6000Action->setChecked(widget->m_trailDurationMs == 6000);

    QMenu *presetMenu = menu->addMenu(uiText("Preset", "プリセット"));
    QAction *redLaserPresetAction = presetMenu->addAction(uiText("Standard", "標準"));
    QAction *blueGlowPresetAction = presetMenu->addAction(uiText("Noticeable", "目立つ"));
    QAction *rainbowPresetAction = presetMenu->addAction(uiText("Follow", "追従"));
    QAction *subtlePresetAction = presetMenu->addAction(uiText("Subtle", "控えめ"));
    QAction *largeSeminarPresetAction = presetMenu->addAction(uiText("Large Seminar", "セミナー大画面"));

    QMenu *resetMenu = menu->addMenu(uiText("Reset", "リセット"));
    QAction *resetDisplayAction = resetMenu->addAction(uiText("Reset Display", "表示設定をリセット"));
    QAction *resetMotionAction = resetMenu->addAction(uiText("Reset Motion", "移動設定をリセット"));
    QAction *resetTrailAction = resetMenu->addAction(uiText("Reset Trail", "軌跡設定をリセット"));
    resetMenu->addSeparator();
    QAction *resetAction = resetMenu->addAction(uiText("Reset All", "すべてリセット"));

    menu->addSeparator();
    QMenu *helpMenu = menu->addMenu(uiText("Help", "ヘルプ"));
    auto addHelpText = [](QMenu *targetMenu, const QString &text, bool header) {
        auto *label = new QLabel(text, targetMenu);
        label->setMargin(0);
        label->setIndent(0);
        label->setStyleSheet(header
                                 ? QStringLiteral("QLabel { color: palette(text); font-weight: 600; padding: 3px 24px 2px 16px; }")
                                 : QStringLiteral("QLabel { color: palette(text); padding: 2px 24px 2px 28px; }"));
        auto *action = new QWidgetAction(targetMenu);
        action->setDefaultWidget(label);
        targetMenu->addAction(action);
    };
    auto addHelpHeader = [&addHelpText](QMenu *targetMenu, const QString &text) {
        targetMenu->addSeparator();
        addHelpText(targetMenu, text, true);
    };
    auto addHelpLine = [&addHelpText](QMenu *targetMenu, const QString &text) {
        addHelpText(targetMenu, text, false);
    };
    if (usesJapaneseHelp()) {
        addHelpHeader(helpMenu, QStringLiteral("マウス"));
        addHelpLine(helpMenu, QStringLiteral("左ドラッグ: ポインターを移動"));
        addHelpLine(helpMenu, QStringLiteral("左クリック: 波紋を表示"));
        addHelpLine(helpMenu, QStringLiteral("マウスホイール: サイズ変更"));
        addHelpLine(helpMenu, QStringLiteral("メニューバーアイコン: メニューを開く"));
        addHelpHeader(helpMenu, QStringLiteral("表示"));
        addHelpLine(helpMenu, QStringLiteral("+ / -: 大きく / 小さく"));
        addHelpLine(helpMenu, QStringLiteral("O / P: 濃く / 薄く"));
        addHelpLine(helpMenu, QStringLiteral("S: 形状を切り替え"));
        addHelpLine(helpMenu, QStringLiteral("Space: 押している間だけ一時拡大"));
        addHelpLine(helpMenu, QStringLiteral("C: 色を選択"));
        addHelpLine(helpMenu, QStringLiteral("V: 虹色"));
        addHelpHeader(helpMenu, QStringLiteral("移動"));
        addHelpLine(helpMenu, QStringLiteral("F: マウスポインターに追従"));
        addHelpLine(helpMenu, QStringLiteral("G: 滑らかに追従"));
        addHelpLine(helpMenu, QStringLiteral("E: 画面内に収める"));
        addHelpHeader(helpMenu, QStringLiteral("点滅と軌跡"));
        addHelpLine(helpMenu, QStringLiteral("B: 点滅"));
        addHelpLine(helpMenu, QStringLiteral(", / .: 点滅を遅く / 速く"));
        addHelpLine(helpMenu, QStringLiteral("T: 軌跡"));
        addHelpLine(helpMenu, QStringLiteral("Y: 軌跡スタイルを切り替え"));
        addHelpLine(helpMenu, QStringLiteral("[ / ]: 軌跡を短く / 長く"));
        addHelpHeader(helpMenu, QStringLiteral("表示制御"));
        addHelpLine(helpMenu, QStringLiteral("A: 自動フェード"));
        addHelpLine(helpMenu, QStringLiteral("M: H を押している間だけ表示するモード"));
        addHelpLine(helpMenu, QStringLiteral("H: Hold H to Show 有効時、押している間だけ表示"));
        addHelpHeader(helpMenu, QStringLiteral("その他"));
        addHelpLine(helpMenu, QStringLiteral("R: リセット"));
        addHelpLine(helpMenu, QStringLiteral("Esc / Q: 終了"));
    } else {
        addHelpHeader(helpMenu, QStringLiteral("Mouse"));
        addHelpLine(helpMenu, QStringLiteral("Left drag: Move the pointer"));
        addHelpLine(helpMenu, QStringLiteral("Left click: Show ripple"));
        addHelpLine(helpMenu, QStringLiteral("Mouse wheel: Resize"));
        addHelpLine(helpMenu, QStringLiteral("Menu bar icon: Open the menu"));
        addHelpHeader(helpMenu, QStringLiteral("Display"));
        addHelpLine(helpMenu, QStringLiteral("+ / -: Larger / Smaller"));
        addHelpLine(helpMenu, QStringLiteral("O / P: More / less opaque"));
        addHelpLine(helpMenu, QStringLiteral("S: Cycle shape"));
        addHelpLine(helpMenu, QStringLiteral("Space: Temporarily enlarge"));
        addHelpLine(helpMenu, QStringLiteral("C: Choose color"));
        addHelpLine(helpMenu, QStringLiteral("V: Rainbow"));
        addHelpHeader(helpMenu, QStringLiteral("Motion"));
        addHelpLine(helpMenu, QStringLiteral("F: Follow cursor"));
        addHelpLine(helpMenu, QStringLiteral("G: Smooth follow"));
        addHelpLine(helpMenu, QStringLiteral("E: Keep on screen"));
        addHelpHeader(helpMenu, QStringLiteral("Blink and Trail"));
        addHelpLine(helpMenu, QStringLiteral("B: Blink"));
        addHelpLine(helpMenu, QStringLiteral(", / .: Slower / faster blink"));
        addHelpLine(helpMenu, QStringLiteral("T: Trail"));
        addHelpLine(helpMenu, QStringLiteral("Y: Cycle trail style"));
        addHelpLine(helpMenu, QStringLiteral("[ / ]: Shorter / longer trail"));
        addHelpHeader(helpMenu, QStringLiteral("Visibility"));
        addHelpLine(helpMenu, QStringLiteral("A: Auto fade"));
        addHelpLine(helpMenu, QStringLiteral("M: Hold H to Show mode"));
        addHelpLine(helpMenu, QStringLiteral("H: Show while held when Hold H to Show is enabled"));
        addHelpHeader(helpMenu, QStringLiteral("Other"));
        addHelpLine(helpMenu, QStringLiteral("R: Reset"));
        addHelpLine(helpMenu, QStringLiteral("Esc / Q: Quit"));
    }
    QAction *quitAction = menu->addAction(uiText("Quit", "終了"));

    QObject::connect(blinkAction, &QAction::triggered, widget, [widget, blinkAction]() {
        widget->setBlinking(blinkAction->isChecked());
    });
    QObject::connect(blinkInterval200Action, &QAction::triggered, widget, [widget]() { widget->setBlinkInterval(200); });
    QObject::connect(blinkInterval500Action, &QAction::triggered, widget, [widget]() { widget->setBlinkInterval(500); });
    QObject::connect(blinkInterval900Action, &QAction::triggered, widget, [widget]() { widget->setBlinkInterval(900); });
    QObject::connect(blinkInterval1200Action, &QAction::triggered, widget, [widget]() { widget->setBlinkInterval(1200); });
    QObject::connect(blinkInterval1600Action, &QAction::triggered, widget, [widget]() { widget->setBlinkInterval(1600); });
    QObject::connect(largerAction, &QAction::triggered, widget, &LaserPointerWidget::increaseSize);
    QObject::connect(smallerAction, &QAction::triggered, widget, &LaserPointerWidget::decreaseSize);
    QObject::connect(opacity25Action, &QAction::triggered, widget, [widget]() { widget->setOpacityPercent(25); });
    QObject::connect(opacity50Action, &QAction::triggered, widget, [widget]() { widget->setOpacityPercent(50); });
    QObject::connect(opacity75Action, &QAction::triggered, widget, [widget]() { widget->setOpacityPercent(75); });
    QObject::connect(opacity100Action, &QAction::triggered, widget, [widget]() { widget->setOpacityPercent(100); });
    QObject::connect(followCursorAction, &QAction::triggered, widget, [widget, followCursorAction]() {
        widget->setFollowCursorEnabled(followCursorAction->isChecked());
    });
    QObject::connect(smoothFollowAction, &QAction::triggered, widget, [widget, smoothFollowAction]() {
        widget->setSmoothFollowEnabled(smoothFollowAction->isChecked());
    });
    QObject::connect(clampAction, &QAction::triggered, widget, [widget, clampAction]() {
        widget->setClampToScreenEnabled(clampAction->isChecked());
    });
    QObject::connect(autoFadeAction, &QAction::triggered, widget, [widget, autoFadeAction]() {
        widget->setAutoFadeEnabled(autoFadeAction->isChecked());
    });
    QObject::connect(holdToShowAction, &QAction::triggered, widget, [widget, holdToShowAction]() {
        widget->setHoldToShowEnabled(holdToShowAction->isChecked());
    });
    QObject::connect(shapeGlowAction, &QAction::triggered, widget, [widget]() { widget->setPointerShape(LaserPointerWidget::PointerShapeGlow); });
    QObject::connect(shapeRingAction, &QAction::triggered, widget, [widget]() { widget->setPointerShape(LaserPointerWidget::PointerShapeRing); });
    QObject::connect(shapeCrossAction, &QAction::triggered, widget, [widget]() { widget->setPointerShape(LaserPointerWidget::PointerShapeCross); });
    QObject::connect(shapeStarAction, &QAction::triggered, widget, [widget]() { widget->setPointerShape(LaserPointerWidget::PointerShapeStar); });
    QObject::connect(rainbowAction, &QAction::triggered, widget, [widget, rainbowAction]() {
        widget->setRainbowEnabled(rainbowAction->isChecked());
    });
    QObject::connect(colorAction, &QAction::triggered, widget, &LaserPointerWidget::chooseColor);
    QObject::connect(trailAction, &QAction::triggered, widget, [widget, trailAction]() {
        widget->setTrailEnabled(trailAction->isChecked());
    });
    QObject::connect(trailStyleGlowAction, &QAction::triggered, widget, [widget]() { widget->setTrailStyle(LaserPointerWidget::TrailStyleGlow); });
    QObject::connect(trailStyleDotAction, &QAction::triggered, widget, [widget]() { widget->setTrailStyle(LaserPointerWidget::TrailStyleDot); });
    QObject::connect(trailDuration500Action, &QAction::triggered, widget, [widget]() { widget->setTrailDuration(500); });
    QObject::connect(trailDuration1000Action, &QAction::triggered, widget, [widget]() { widget->setTrailDuration(1000); });
    QObject::connect(trailDuration1500Action, &QAction::triggered, widget, [widget]() { widget->setTrailDuration(1500); });
    QObject::connect(trailDuration2000Action, &QAction::triggered, widget, [widget]() { widget->setTrailDuration(2000); });
    QObject::connect(trailDuration2500Action, &QAction::triggered, widget, [widget]() { widget->setTrailDuration(2500); });
    QObject::connect(trailDuration3000Action, &QAction::triggered, widget, [widget]() { widget->setTrailDuration(3000); });
    QObject::connect(trailDuration4000Action, &QAction::triggered, widget, [widget]() { widget->setTrailDuration(4000); });
    QObject::connect(trailDuration5000Action, &QAction::triggered, widget, [widget]() { widget->setTrailDuration(5000); });
    QObject::connect(trailDuration6000Action, &QAction::triggered, widget, [widget]() { widget->setTrailDuration(6000); });
    QObject::connect(redLaserPresetAction, &QAction::triggered, widget, [widget]() { widget->applyPreset(LaserPointerWidget::PresetRedLaser); });
    QObject::connect(blueGlowPresetAction, &QAction::triggered, widget, [widget]() { widget->applyPreset(LaserPointerWidget::PresetBlueGlow); });
    QObject::connect(rainbowPresetAction, &QAction::triggered, widget, [widget]() { widget->applyPreset(LaserPointerWidget::PresetRainbow); });
    QObject::connect(subtlePresetAction, &QAction::triggered, widget, [widget]() { widget->applyPreset(LaserPointerWidget::PresetSubtle); });
    QObject::connect(largeSeminarPresetAction, &QAction::triggered, widget, [widget]() { widget->applyPreset(LaserPointerWidget::PresetLargeSeminar); });
    QObject::connect(resetDisplayAction, &QAction::triggered, widget, &LaserPointerWidget::resetDisplaySettings);
    QObject::connect(resetMotionAction, &QAction::triggered, widget, &LaserPointerWidget::resetMotionSettings);
    QObject::connect(resetTrailAction, &QAction::triggered, widget, &LaserPointerWidget::resetTrailSettings);
    QObject::connect(resetAction, &QAction::triggered, widget, &LaserPointerWidget::resetToDefaults);
    QObject::connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
}
