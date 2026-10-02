#pragma once

#include <QColor>

class LaserPointerSettings
{
public:
    struct Values {
        QColor color;
        bool autoFadeEnabled;
        bool clampToScreenEnabled;
        bool holdToShowEnabled;
        int opacityPercent;
        int pointerShape;
        bool rainbowEnabled;
        int pointerSize;
        bool followCursorEnabled;
        bool smoothFollowEnabled;
        bool blinking;
        int blinkIntervalMs;
        bool trailEnabled;
        int trailStyle;
        int trailDurationMs;
        bool firstLaunchNotice;
    };

    static Values load();
    static void save(const Values &values);
};
