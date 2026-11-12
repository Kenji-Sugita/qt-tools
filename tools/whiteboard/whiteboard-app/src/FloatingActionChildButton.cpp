#include "FloatingActionChildButton.h"

#include "FloatingActionBubbleLabel.h"

#include <QEvent>
#include <QHideEvent>
#include <QMoveEvent>
#include <QResizeEvent>

FloatingActionChildButton::FloatingActionChildButton(QWidget *parent, QWidget *labelParent)
    : QToolButton(parent)
    , m_labelParent(labelParent ? labelParent : parent)
{
}

void FloatingActionChildButton::setBubbleText(const QString &text)
{
    m_bubbleText = text;
    if (m_bubbleLabel)
        m_bubbleLabel->setText(text);
}

void FloatingActionChildButton::enterEvent(QEnterEvent *event)
{
    QToolButton::enterEvent(event);
    showHoverState();
}

void FloatingActionChildButton::leaveEvent(QEvent *event)
{
    QToolButton::leaveEvent(event);
    hideHoverState();
}

void FloatingActionChildButton::hideEvent(QHideEvent *event)
{
    hideHoverState();
    QToolButton::hideEvent(event);
}

void FloatingActionChildButton::moveEvent(QMoveEvent *event)
{
    QToolButton::moveEvent(event);
    updateBubbleGeometry();
}

void FloatingActionChildButton::resizeEvent(QResizeEvent *event)
{
    QToolButton::resizeEvent(event);
    updateBubbleGeometry();
}

void FloatingActionChildButton::ensureBubble()
{
    if (m_bubbleLabel || !m_labelParent)
        return;

    m_bubbleLabel = new FloatingActionBubbleLabel(m_labelParent);
    m_bubbleLabel->setObjectName(QStringLiteral("floatingActionBubbleLabel"));
    m_bubbleLabel->hide();
    if (!m_bubbleText.isEmpty())
        m_bubbleLabel->setText(m_bubbleText);
}

void FloatingActionChildButton::showHoverState()
{
    if (m_hovered || !isEnabled())
        return;

    m_hovered = true;
    raise();

    ensureBubble();
    if (m_bubbleLabel) {
        updateBubbleGeometry();
        m_bubbleLabel->show();
        m_bubbleLabel->raise();
        raise();
    }
}

void FloatingActionChildButton::hideHoverState()
{
    if (!m_hovered)
        return;

    m_hovered = false;
    if (m_bubbleLabel)
        m_bubbleLabel->hide();
}

void FloatingActionChildButton::updateBubbleGeometry()
{
    if (!m_hovered || !m_bubbleLabel || !m_labelParent)
        return;

    const QPoint buttonTopLeft = m_labelParent->mapFromGlobal(mapToGlobal(QPoint(0, 0)));
    const QSize hint = m_bubbleLabel->sizeHint();
    const int availableWidth = qMax(72, buttonTopLeft.x() - 14);
    const int labelWidth = qBound(72, hint.width(), availableWidth);
    const int labelHeight = qMax(28, hint.height());
    const int labelX = qMax(0, buttonTopLeft.x() - labelWidth - 6);
    const int labelY = buttonTopLeft.y() + ((height() - labelHeight) / 2);
    m_bubbleLabel->setGeometry(labelX, labelY, labelWidth, labelHeight);
}
