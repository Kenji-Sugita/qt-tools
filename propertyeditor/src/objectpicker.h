#pragma once

#include "propertyeditorglobal.h"

#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QPointer>

class QEvent;
class QWidget;
class ObjectHighlighter;

class PROPERTY_EDITOR_EXPORT ObjectPicker : public QObject
{
    Q_OBJECT

public:
    explicit ObjectPicker(QObject *parentObject = nullptr);
    ~ObjectPicker() override;

    bool start(QWidget *rootWidget);
    void stop();
    void setSelectedObject(QObject *targetObject);

    bool isActive() const;
    QWidget *rootWidget() const;
    QWidget *hoveredWidget() const;
    QObject *selectedObject() const;

signals:
    void objectPicked(QObject *targetObject);
    void hoveredObjectChanged(QObject *targetObject);
    void activeChanged(bool active);

protected:
    bool eventFilter(QObject *watchedObject, QEvent *event) override;

private:
    void stop(bool clearHover);
    bool isObjectInsideRoot(const QObject *object) const;
    void updateOverlayGeometry();
    QWidget *deepestChildAt(QWidget *rootWidget, const QPoint &positionInRoot) const;
    void updateHover(const QPoint &globalPosition);
    void setHoveredWidget(QWidget *widget);

    QPointer<QWidget> rootWidgetPointer;
    QPointer<QWidget> hoveredWidgetPointer;
    QPointer<QWidget> inputOverlayWidget;
    QPointer<QObject> selectedObjectPointer;
    ObjectHighlighter *hoverHighlighter = nullptr;
    ObjectHighlighter *selectedHighlighter = nullptr;
    bool active = false;
    bool hasConfirmedSelectionInCurrentSession = false;
};
