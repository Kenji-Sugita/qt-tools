#include "objecthighlighter.h"

#include <QtGui/QColor>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QLayout>
#include <QtWidgets/QWidget>

namespace {

class HighlightOverlayWidget : public QWidget
{
public:
    enum class HighlightMode {
        WidgetFill,
        LayoutOutline
    };

    explicit HighlightOverlayWidget(QWidget *parentWidget = nullptr)
        : QWidget(parentWidget)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
        setProperty("objectHighlighterOverlay", true);
    }

    void setHighlightMode(HighlightMode newHighlightMode)
    {
        highlightMode = newHighlightMode;
        update();
    }

protected:
    void paintEvent(QPaintEvent *paintEvent) override
    {
        Q_UNUSED(paintEvent);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, false);
        if (highlightMode == HighlightMode::WidgetFill) {
            painter.fillRect(rect(), QColor(255, 180, 180, 64));
            painter.setPen(QPen(QColor(255, 140, 140, 200), 2));
        } else {
            painter.setPen(QPen(QColor(255, 0, 0, 220), 2));
        }
        painter.drawRect(rect().adjusted(1, 1, -1, -1));
    }

private:
    HighlightMode highlightMode = HighlightMode::WidgetFill;
};

} // namespace

ObjectHighlighter::ObjectHighlighter(QObject *parentObject)
    : QObject(parentObject)
{
}

ObjectHighlighter::~ObjectHighlighter()
{
    clear();
}

void ObjectHighlighter::setTargetObject(QObject *targetObject)
{
    targetObjectPointer = targetObject;
    updateHighlight();
}

QObject *ObjectHighlighter::targetObject() const
{
    return targetObjectPointer;
}

void ObjectHighlighter::clear()
{
    targetObjectPointer.clear();
    if (highlightOverlay != nullptr) {
        highlightOverlay->hide();
        delete highlightOverlay;
        highlightOverlay = nullptr;
    }

    highlightedWindowWidget.clear();
}

void ObjectHighlighter::updateHighlight()
{
    QWidget *targetWidget = qobject_cast<QWidget *>(targetObjectPointer);
    QLayout *targetLayout = qobject_cast<QLayout *>(targetObjectPointer);
    QWidget *referenceWidget = targetWidget;
    QRect highlightGeometry;
    HighlightOverlayWidget::HighlightMode highlightMode =
        HighlightOverlayWidget::HighlightMode::WidgetFill;

    if (targetLayout != nullptr) {
        referenceWidget = targetLayout->parentWidget();
        if (referenceWidget != nullptr) {
            highlightGeometry = targetLayout->geometry();
            highlightMode = HighlightOverlayWidget::HighlightMode::LayoutOutline;
        }
    } else if (targetWidget != nullptr) {
        highlightGeometry = QRect(QPoint(0, 0), targetWidget->size());
    }

    if (referenceWidget == nullptr || !referenceWidget->isVisible()) {
        clear();
        return;
    }

    QWidget *windowWidget = referenceWidget->window();
    if (windowWidget == nullptr || !windowWidget->isVisible()) {
        clear();
        return;
    }

    if (highlightOverlay == nullptr || highlightedWindowWidget != windowWidget) {
        if (highlightOverlay != nullptr) {
            highlightOverlay->hide();
            delete highlightOverlay;
            highlightOverlay = nullptr;
        }

        highlightedWindowWidget = windowWidget;
        highlightOverlay = new HighlightOverlayWidget(windowWidget);
    }

    if (targetLayout == nullptr) {
        highlightGeometry.moveTopLeft(referenceWidget->mapTo(windowWidget, QPoint(0, 0)));
    } else {
        highlightGeometry.moveTopLeft(
            referenceWidget->mapTo(windowWidget, highlightGeometry.topLeft()));
    }

    if (!highlightGeometry.isValid() || highlightGeometry.isEmpty()) {
        clear();
        return;
    }

    static_cast<HighlightOverlayWidget *>(highlightOverlay)->setHighlightMode(highlightMode);
    highlightOverlay->setGeometry(highlightGeometry);
    highlightOverlay->raise();
    highlightOverlay->show();
}
