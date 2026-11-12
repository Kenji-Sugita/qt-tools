#pragma once

class LaserPointerWidget;
class QMenu;
class QString;

class LaserPointerMenuBuilder
{
public:
    static void populate(QMenu *menu, LaserPointerWidget *widget, bool includePointerVisibilityAction);

private:
    static QString pointerShapeName(int shape);
    static QString trailStyleName(int style);
};
