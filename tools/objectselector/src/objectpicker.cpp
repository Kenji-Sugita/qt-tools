#include "objectpicker.h"

#include "objecthighlighter.h"

#include <QtCore/QEvent>
#include <QtGui/QCursor>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

#include <functional>

namespace {

QWidget *focusTargetForPickedWidget(QWidget *pickedWidget)
{
    QWidget *focusTarget = pickedWidget;
    while (focusTarget != nullptr) {
        if (focusTarget->focusPolicy() != Qt::NoFocus) {
            return focusTarget;
        }
        focusTarget = focusTarget->parentWidget();
    }

    return pickedWidget;
}

class ObjectPickerOverlay : public QWidget
{
public:
    explicit ObjectPickerOverlay(QWidget *parentWidget = nullptr)
        : QWidget(parentWidget)
    {
        setMouseTracking(true);
        setFocusPolicy(Qt::NoFocus);
        setCursor(Qt::CrossCursor);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
    }

    std::function<void(const QPoint &)> mouseMoveHandler;
    std::function<void(const QPoint &, Qt::MouseButton)> mousePressHandler;
    std::function<void()> leaveHandler;

protected:
    void mouseMoveEvent(QMouseEvent *mouseEvent) override
    {
        if (mouseMoveHandler) {
            mouseMoveHandler(mouseEvent->globalPosition().toPoint());
        }
        mouseEvent->accept();
    }

    void mousePressEvent(QMouseEvent *mouseEvent) override
    {
        if (mousePressHandler) {
            mousePressHandler(mouseEvent->globalPosition().toPoint(), mouseEvent->button());
        }
        mouseEvent->accept();
    }

    void mouseDoubleClickEvent(QMouseEvent *mouseEvent) override
    {
        mousePressEvent(mouseEvent);
    }

    void mouseReleaseEvent(QMouseEvent *mouseEvent) override
    {
        mouseEvent->accept();
    }

    void wheelEvent(QWheelEvent *wheelEvent) override
    {
        wheelEvent->accept();
    }

    void leaveEvent(QEvent *event) override
    {
        if (leaveHandler) {
            leaveHandler();
        }
        QWidget::leaveEvent(event);
    }

    void paintEvent(QPaintEvent *paintEvent) override
    {
        Q_UNUSED(paintEvent)
    }
};

} // namespace

ObjectPicker::ObjectPicker(QObject *parentObject)
    : QObject(parentObject)
{
    hoverHighlighter = new ObjectHighlighter(this);
    selectedHighlighter = new ObjectHighlighter(this);
}

ObjectPicker::~ObjectPicker()
{
    stop();
}

bool ObjectPicker::start(QWidget *rootWidget)
{
    if (active) {
        return false;
    }

    if (rootWidget == nullptr || !rootWidget->isVisible()) {
        return false;
    }

    rootWidgetPointer = rootWidget;
    active = true;
    hasConfirmedSelectionInCurrentSession = false;
    hoveredWidgetPointer.clear();
    if (selectedHighlighter != nullptr) {
        selectedHighlighter->clear();
    }

    if (QWidget *focusedWidget = qApp->focusWidget();
        focusedWidget != nullptr && isObjectInsideRoot(focusedWidget)) {
        focusedWidget->clearFocus();
    }

    auto *inputOverlay = new ObjectPickerOverlay(rootWidget);
    inputOverlayWidget = inputOverlay;
    inputOverlay->mouseMoveHandler = [this](const QPoint &globalPosition) {
        updateHover(globalPosition);
    };
    inputOverlay->mousePressHandler = [this](const QPoint &globalPosition, Qt::MouseButton mouseButton) {
        updateHover(globalPosition);
        if (mouseButton == Qt::RightButton) {
            stop();
            return;
        }

        if (hoveredWidgetPointer != nullptr && mouseButton == Qt::LeftButton) {
            QWidget *pickedWidget = hoveredWidgetPointer;
            hasConfirmedSelectionInCurrentSession = true;
            setSelectedObject(pickedWidget);
            emit objectPicked(pickedWidget);
            stop(false);
            if (QWidget *focusTarget = focusTargetForPickedWidget(pickedWidget);
                focusTarget != nullptr && focusTarget->focusPolicy() != Qt::NoFocus) {
                focusTarget->setFocus(Qt::OtherFocusReason);
            }
        }
    };
    inputOverlay->leaveHandler = [this]() {
        setHoveredWidget(nullptr);
    };
    updateOverlayGeometry();
    inputOverlay->raise();
    inputOverlay->show();

    qApp->installEventFilter(this);
    qApp->setOverrideCursor(Qt::CrossCursor);
    emit activeChanged(true);
    updateHover(QCursor::pos());
    return true;
}

void ObjectPicker::stop()
{
    stop(true);
}

void ObjectPicker::setSelectedObject(QObject *targetObject)
{
    selectedObjectPointer = targetObject;
    if (!active && selectedHighlighter != nullptr) {
        selectedHighlighter->setTargetObject(targetObject);
    }
}

bool ObjectPicker::isActive() const
{
    return active;
}

QWidget *ObjectPicker::rootWidget() const
{
    return rootWidgetPointer;
}

QWidget *ObjectPicker::hoveredWidget() const
{
    return hoveredWidgetPointer;
}

QObject *ObjectPicker::selectedObject() const
{
    return selectedObjectPointer;
}

bool ObjectPicker::eventFilter(QObject *watchedObject, QEvent *event)
{
    if (!active) {
        return QObject::eventFilter(watchedObject, event);
    }

    if (rootWidgetPointer == nullptr) {
        stop();
        return QObject::eventFilter(watchedObject, event);
    }

    QWidget *pickerWindow = rootWidgetPointer->window();
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            stop();
            return true;
        }
    }

    if ((event->type() == QEvent::KeyPress
         || event->type() == QEvent::KeyRelease
         || event->type() == QEvent::ShortcutOverride
         || event->type() == QEvent::InputMethod)
        && isObjectInsideRoot(qApp->focusWidget())) {
        return true;
    }

    if (event->type() == QEvent::FocusIn && isObjectInsideRoot(watchedObject)) {
        return true;
    }

    if (watchedObject == rootWidgetPointer.data() && event->type() == QEvent::Resize) {
        updateOverlayGeometry();
    } else if (event->type() == QEvent::Hide || event->type() == QEvent::Close
               || event->type() == QEvent::Destroy) {
        if (watchedObject == rootWidgetPointer.data() || watchedObject == pickerWindow) {
            stop();
        }
    }

    return QObject::eventFilter(watchedObject, event);
}

void ObjectPicker::stop(bool clearHover)
{
    if (active) {
        qApp->removeEventFilter(this);
        qApp->restoreOverrideCursor();
    }

    if (!hasConfirmedSelectionInCurrentSession) {
        selectedObjectPointer.clear();
    }

    active = false;
    hasConfirmedSelectionInCurrentSession = false;
    rootWidgetPointer.clear();
    if (inputOverlayWidget != nullptr) {
        inputOverlayWidget->deleteLater();
        inputOverlayWidget = nullptr;
    }
    if (hoverHighlighter != nullptr) {
        hoverHighlighter->clear();
    }
    if (selectedHighlighter != nullptr) {
        selectedHighlighter->setTargetObject(selectedObjectPointer);
    }

    if (clearHover) {
        setHoveredWidget(nullptr);
    } else {
        hoveredWidgetPointer.clear();
    }

    emit activeChanged(false);
}

bool ObjectPicker::isObjectInsideRoot(const QObject *object) const
{
    if (object == nullptr || rootWidgetPointer == nullptr) {
        return false;
    }

    const QObject *currentObject = object;
    while (currentObject != nullptr) {
        if (currentObject == rootWidgetPointer) {
            return true;
        }
        currentObject = currentObject->parent();
    }

    return false;
}

void ObjectPicker::updateOverlayGeometry()
{
    if (inputOverlayWidget == nullptr || rootWidgetPointer == nullptr) {
        return;
    }

    inputOverlayWidget->setGeometry(rootWidgetPointer->rect());
}

QWidget *ObjectPicker::deepestChildAt(QWidget *rootWidget, const QPoint &positionInRoot) const
{
    if (rootWidget == nullptr || !rootWidget->isVisible()) {
        return nullptr;
    }

    if (!QRect(QPoint(0, 0), rootWidget->size()).contains(positionInRoot)) {
        return nullptr;
    }

    const bool shouldRestoreOverlayVisibility =
        inputOverlayWidget != nullptr && inputOverlayWidget->isVisible();
    if (shouldRestoreOverlayVisibility) {
        inputOverlayWidget->hide();
    }

    QWidget *currentWidget = rootWidget;
    QPoint currentPosition = positionInRoot;

    while (currentWidget != nullptr) {
        QWidget *childWidget = currentWidget->childAt(currentPosition);
        if (childWidget == nullptr || childWidget == currentWidget || !childWidget->isVisible()) {
            if (shouldRestoreOverlayVisibility) {
                inputOverlayWidget->show();
                inputOverlayWidget->raise();
            }
            return currentWidget;
        }

        currentPosition = childWidget->mapFrom(currentWidget, currentPosition);
        currentWidget = childWidget;
    }

    if (shouldRestoreOverlayVisibility) {
        inputOverlayWidget->show();
        inputOverlayWidget->raise();
    }
    return rootWidget;
}

void ObjectPicker::updateHover(const QPoint &globalPosition)
{
    if (!active || rootWidgetPointer == nullptr || !rootWidgetPointer->isVisible()) {
        stop();
        return;
    }

    const QPoint positionInRoot = rootWidgetPointer->mapFromGlobal(globalPosition);
    setHoveredWidget(deepestChildAt(rootWidgetPointer, positionInRoot));
}

void ObjectPicker::setHoveredWidget(QWidget *widget)
{
    if (hoveredWidgetPointer == widget) {
        return;
    }

    hoveredWidgetPointer = widget;
    if (hoverHighlighter != nullptr) {
        hoverHighlighter->setTargetObject(widget);
    }
    emit hoveredObjectChanged(widget);
}
