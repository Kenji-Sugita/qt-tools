#include "LaserPointerSettings.h"

#include <QSettings>

#include <algorithm>

namespace {
constexpr int kMinimumPointerSize = 24;
constexpr int kMaximumPointerSize = 260;
constexpr int kDefaultPointerSize = 96;
constexpr int kMinimumBlinkIntervalMs = 200;
constexpr int kMaximumBlinkIntervalMs = 1600;
constexpr int kDefaultBlinkIntervalMs = 900;
constexpr int kMinimumTrailDurationMs = 500;
constexpr int kMaximumTrailDurationMs = 6000;
constexpr int kDefaultTrailDurationMs = 2500;
constexpr int kMinimumOpacityPercent = 25;
constexpr int kMaximumOpacityPercent = 100;
constexpr int kDefaultOpacityPercent = 100;
constexpr int kPointerShapeCount = 4;
constexpr int kTrailStyleCount = 2;
const QString kSettingsColorKey = QStringLiteral("pointer/color");
const QString kSettingsAutoFadeEnabledKey = QStringLiteral("pointer/autoFadeEnabled");
const QString kSettingsClampToScreenEnabledKey = QStringLiteral("pointer/clampToScreenEnabled");
const QString kSettingsHoldToShowEnabledKey = QStringLiteral("pointer/holdToShowEnabled");
const QString kSettingsOpacityPercentKey = QStringLiteral("pointer/opacityPercent");
const QString kSettingsPointerShapeKey = QStringLiteral("pointer/shape");
const QString kSettingsRainbowEnabledKey = QStringLiteral("pointer/rainbowEnabled");
const QString kSettingsPointerSizeKey = QStringLiteral("pointer/size");
const QString kSettingsBlinkingKey = QStringLiteral("blink/enabled");
const QString kSettingsBlinkIntervalKey = QStringLiteral("blink/intervalMs");
const QString kSettingsFollowCursorEnabledKey = QStringLiteral("pointer/followCursorEnabled");
const QString kSettingsSmoothFollowEnabledKey = QStringLiteral("pointer/smoothFollowEnabled");
const QString kSettingsTrailEnabledKey = QStringLiteral("trail/enabled");
const QString kSettingsTrailDurationKey = QStringLiteral("trail/durationMs");
const QString kSettingsTrailStyleKey = QStringLiteral("trail/style");
const QString kSettingsInitializedKey = QStringLiteral("app/initialized");
const QColor kDefaultColor(255, 24, 24);
} // namespace

LaserPointerSettings::Values LaserPointerSettings::load()
{
    QSettings settings;

    Values values;
    values.firstLaunchNotice = !settings.value(kSettingsInitializedKey, false).toBool();
    if (values.firstLaunchNotice) {
        settings.setValue(kSettingsInitializedKey, true);
    }

    const QColor savedColor = settings.value(kSettingsColorKey, kDefaultColor).value<QColor>();
    values.color = savedColor.isValid() ? savedColor : kDefaultColor;
    values.autoFadeEnabled = settings.value(kSettingsAutoFadeEnabledKey, false).toBool();
    values.clampToScreenEnabled = settings.value(kSettingsClampToScreenEnabledKey, false).toBool();
    values.holdToShowEnabled = settings.value(kSettingsHoldToShowEnabledKey, false).toBool();
    values.opacityPercent = std::clamp(settings.value(kSettingsOpacityPercentKey,
                                                      kDefaultOpacityPercent)
                                           .toInt(),
                                       kMinimumOpacityPercent,
                                       kMaximumOpacityPercent);
    values.pointerShape = std::clamp(settings.value(kSettingsPointerShapeKey, 0).toInt(),
                                     0,
                                     kPointerShapeCount - 1);
    values.rainbowEnabled = settings.value(kSettingsRainbowEnabledKey, false).toBool();
    values.pointerSize = std::clamp(settings.value(kSettingsPointerSizeKey, kDefaultPointerSize).toInt(),
                                    kMinimumPointerSize,
                                    kMaximumPointerSize);
    values.followCursorEnabled = settings.value(kSettingsFollowCursorEnabledKey, false).toBool();
    values.smoothFollowEnabled = settings.value(kSettingsSmoothFollowEnabledKey, false).toBool();
    values.blinking = settings.value(kSettingsBlinkingKey, false).toBool();
    values.blinkIntervalMs = std::clamp(settings.value(kSettingsBlinkIntervalKey,
                                                       kDefaultBlinkIntervalMs)
                                            .toInt(),
                                        kMinimumBlinkIntervalMs,
                                        kMaximumBlinkIntervalMs);
    values.trailEnabled = settings.value(kSettingsTrailEnabledKey, false).toBool();
    values.trailStyle = std::clamp(settings.value(kSettingsTrailStyleKey, 0).toInt(),
                                   0,
                                   kTrailStyleCount - 1);
    values.trailDurationMs = std::clamp(settings.value(kSettingsTrailDurationKey,
                                                       kDefaultTrailDurationMs)
                                            .toInt(),
                                        kMinimumTrailDurationMs,
                                        kMaximumTrailDurationMs);
    return values;
}

void LaserPointerSettings::save(const Values &values)
{
    QSettings settings;
    settings.setValue(kSettingsColorKey, values.color);
    settings.setValue(kSettingsAutoFadeEnabledKey, values.autoFadeEnabled);
    settings.setValue(kSettingsClampToScreenEnabledKey, values.clampToScreenEnabled);
    settings.setValue(kSettingsHoldToShowEnabledKey, values.holdToShowEnabled);
    settings.setValue(kSettingsOpacityPercentKey, values.opacityPercent);
    settings.setValue(kSettingsPointerShapeKey, values.pointerShape);
    settings.setValue(kSettingsRainbowEnabledKey, values.rainbowEnabled);
    settings.setValue(kSettingsPointerSizeKey, values.pointerSize);
    settings.setValue(kSettingsFollowCursorEnabledKey, values.followCursorEnabled);
    settings.setValue(kSettingsSmoothFollowEnabledKey, values.smoothFollowEnabled);
    settings.setValue(kSettingsBlinkingKey, values.blinking);
    settings.setValue(kSettingsBlinkIntervalKey, values.blinkIntervalMs);
    settings.setValue(kSettingsTrailEnabledKey, values.trailEnabled);
    settings.setValue(kSettingsTrailStyleKey, values.trailStyle);
    settings.setValue(kSettingsTrailDurationKey, values.trailDurationMs);
    settings.setValue(kSettingsInitializedKey, true);
}
