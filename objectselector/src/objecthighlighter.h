#pragma once

#include "objectselectorglobal.h"

#include <QtCore/QPointer>
#include <QtCore/QObject>

class QWidget;

class OBJECT_SELECTOR_EXPORT ObjectHighlighter : public QObject
{
    Q_OBJECT

public:
    explicit ObjectHighlighter(QObject *parentObject = nullptr);
    ~ObjectHighlighter() override;

    void setTargetObject(QObject *targetObject);
    QObject *targetObject() const;
    void clear();

private:
    void updateHighlight();

    QPointer<QObject> targetObjectPointer;
    QPointer<QWidget> highlightedWindowWidget;
    QWidget *highlightOverlay = nullptr;
};
