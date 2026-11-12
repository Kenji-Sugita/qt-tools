#include "PersistentToolBar.h"

#include <QAction>
#include <QFrame>
#include <QHideEvent>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QResizeEvent>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace {
constexpr int ToolbarSpacing = 5;
constexpr int PanelMargin = 8;
}

PersistentToolBar::PersistentToolBar(QWidget *overlayParent, QWidget *parent)
    : QWidget(parent)
    , m_overlayParent(overlayParent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumWidth(0);

    m_primaryLayout = new QHBoxLayout(this);
    m_primaryLayout->setContentsMargins(0, 0, 0, 0);
    m_primaryLayout->setSpacing(ToolbarSpacing);

    m_extensionButton = new QToolButton(this);
    m_extensionButton->setObjectName(QStringLiteral("persistentToolbarExtensionButton"));
    m_extensionButton->setCheckable(true);
    m_extensionButton->setAutoRaise(false);
    m_extensionButton->setMinimumSize(38, 38);
    m_extensionButton->setToolTip(tr("More Tools"));
    m_extensionButton->setAccessibleName(tr("More Tools"));
    configureItemWidget(m_extensionButton);
    m_extensionButton->hide();
    connect(m_extensionButton, &QToolButton::clicked, this, [this](bool checked) {
        setOverflowExpanded(checked);
    });

    m_overflowPanel = new QFrame(overlayParent);
    m_overflowPanel->setObjectName(QStringLiteral("persistentToolbarOverflowPanel"));
    m_overflowPanel->setFrameShape(QFrame::StyledPanel);
    m_overflowPanel->hide();

    m_overflowLayout = new QVBoxLayout(m_overflowPanel);
    m_overflowLayout->setContentsMargins(0, PanelMargin, 0, PanelMargin);
    m_overflowLayout->setSpacing(ToolbarSpacing);

    updateExtensionButton();
}

void PersistentToolBar::addAction(QAction *action)
{
    if (!action)
        return;

    QWidget::addAction(action);
    auto *button = new QToolButton(this);
    button->setDefaultAction(action);
    button->setIconSize(m_iconSize);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setMinimumSize(38, 38);
    configureItemWidget(button);
    m_items.append({button, false});
    m_actionWidgets.insert(action, button);
    refreshOverflow();
}

void PersistentToolBar::addSeparator()
{
    auto *separator = new QFrame(this);
    separator->setObjectName(QStringLiteral("persistentToolbarSeparator"));
    separator->setFrameShape(QFrame::VLine);
    separator->setFixedWidth(15);
    separator->setMinimumHeight(34);
    configureItemWidget(separator);
    m_items.append({separator, true});
    refreshOverflow();
}

void PersistentToolBar::addWidget(QWidget *widget)
{
    if (!widget)
        return;
    widget->setParent(this);
    configureItemWidget(widget);
    m_items.append({widget, false});
    refreshOverflow();
}

QWidget *PersistentToolBar::widgetForAction(QAction *action) const
{
    return m_actionWidgets.value(action, nullptr);
}

void PersistentToolBar::setIconSize(const QSize &iconSize)
{
    if (!iconSize.isValid() || iconSize == m_iconSize)
        return;
    m_iconSize = iconSize;
    for (QWidget *widget : m_actionWidgets) {
        if (auto *button = qobject_cast<QToolButton *>(widget))
            button->setIconSize(iconSize);
    }
    refreshOverflow();
}

void PersistentToolBar::setOverflowStyleSheet(const QString &styleSheet)
{
    if (!m_overflowPanel)
        return;
    m_overflowPanel->setStyleSheet(
        styleSheet
        + QStringLiteral(
            "QFrame#persistentToolbarOverflowPanel {"
            "background: #3f464d;"
            "border: 1px solid #242a30;"
            "border-radius: 6px;"
            "}"));
}

bool PersistentToolBar::hasOverflow() const
{
    return m_firstOverflowIndex >= 0;
}

bool PersistentToolBar::overflowExpanded() const
{
    return m_overflowPanel && m_overflowPanel->isVisible();
}

void PersistentToolBar::setOverflowExpanded(bool expanded)
{
    expanded = expanded && hasOverflow();
    m_extensionButton->setChecked(expanded);
    updateExtensionButton();
    if (!expanded) {
        m_overflowPanel->hide();
        return;
    }

    populateOverflowPanel(m_firstOverflowIndex);
    positionOverflowPanel();
    m_overflowPanel->show();
    m_overflowPanel->raise();
}

void PersistentToolBar::refreshOverflow()
{
    if (m_refreshing)
        return;
    m_refreshing = true;

    const bool wasExpanded = overflowExpanded();
    clearLayout(m_primaryLayout);
    clearLayout(m_overflowLayout);

    const int spacing = m_primaryLayout->spacing();
    int totalWidth = 0;
    for (int i = 0; i < m_items.size(); ++i) {
        if (i > 0)
            totalWidth += spacing;
        totalWidth += itemWidth(m_items.at(i));
    }

    const int extensionWidth = std::max(m_extensionButton->sizeHint().width(), 36);
    const int availableWidth = std::max(0, width());
    int visibleCount = m_items.size();
    if (totalWidth > availableWidth) {
        const int availableForItems = std::max(0, availableWidth - extensionWidth - spacing);
        int usedWidth = 0;
        visibleCount = 0;
        for (const Item &item : std::as_const(m_items)) {
            const int nextWidth = itemWidth(item) + (visibleCount > 0 ? spacing : 0);
            if (visibleCount > 0 && usedWidth + nextWidth > availableForItems)
                break;
            if (visibleCount == 0 && itemWidth(item) > availableForItems)
                break;
            usedWidth += nextWidth;
            ++visibleCount;
        }
        while (visibleCount > 0 && m_items.at(visibleCount - 1).separator)
            --visibleCount;
    }

    m_firstOverflowIndex = visibleCount < m_items.size() ? visibleCount : -1;
    for (int i = 0; i < visibleCount; ++i) {
        QWidget *widget = m_items.at(i).widget;
        widget->setParent(this);
        widget->show();
        m_primaryLayout->addWidget(widget);
    }

    if (hasOverflow()) {
        m_extensionButton->setParent(this);
        m_extensionButton->show();
        m_primaryLayout->addWidget(m_extensionButton);
        populateOverflowPanel(m_firstOverflowIndex);
    } else {
        m_extensionButton->hide();
        m_extensionButton->setChecked(false);
        m_overflowPanel->hide();
    }

    updateExtensionButton();
    m_refreshing = false;

    if (wasExpanded && hasOverflow())
        setOverflowExpanded(true);
}

QSize PersistentToolBar::sizeHint() const
{
    int height = m_extensionButton ? m_extensionButton->sizeHint().height() : 36;
    for (const Item &item : m_items)
        height = std::max(height, item.widget->sizeHint().height());
    return QSize(200, height);
}

QSize PersistentToolBar::minimumSizeHint() const
{
    return QSize(36, sizeHint().height());
}

void PersistentToolBar::hideEvent(QHideEvent *event)
{
    setOverflowExpanded(false);
    QWidget::hideEvent(event);
}

void PersistentToolBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    refreshOverflow();
}

void PersistentToolBar::clearLayout(QLayout *layout)
{
    if (!layout)
        return;
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QLayout *childLayout = item->layout())
            clearLayout(childLayout);
        delete item;
    }
}

void PersistentToolBar::configureItemWidget(QWidget *widget)
{
#ifdef Q_OS_MACOS
    if (widget)
        widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
#else
    Q_UNUSED(widget);
#endif
}

int PersistentToolBar::itemWidth(const Item &item) const
{
    if (!item.widget)
        return 0;
    const int preferredWidth = std::max({item.widget->minimumWidth(),
                                         item.widget->minimumSizeHint().width(),
                                         item.widget->sizeHint().width()});
    return std::min(preferredWidth, item.widget->maximumWidth());
}

int PersistentToolBar::overflowPanelLeftInset() const
{
    if (!m_overlayParent || !parentWidget())
        return 0;

    const int contentLeft = mapTo(m_overlayParent, QPoint(0, 0)).x();
    const int toolbarLeft = parentWidget()->mapTo(m_overlayParent, QPoint(0, 0)).x();
    return std::max(0, contentLeft - toolbarLeft);
}

void PersistentToolBar::populateOverflowPanel(int firstOverflowIndex)
{
    if (firstOverflowIndex < 0 || !m_overlayParent)
        return;

    clearLayout(m_overflowLayout);
    const int leftInset = overflowPanelLeftInset();
    const int rightInset = leftInset;
    m_overflowLayout->setContentsMargins(leftInset, PanelMargin, rightInset, PanelMargin);
    const int availableWidth = std::max(200, m_overlayParent->width() - 2 * PanelMargin);
    auto createRowLayout = [this]() {
        auto *layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(ToolbarSpacing);
        layout->setAlignment(Qt::AlignLeft);
        m_overflowLayout->addLayout(layout);
        return layout;
    };
    QHBoxLayout *rowLayout = createRowLayout();
    int itemCountInRow = 0;
    int rowWidth = leftInset + rightInset;
    bool placedItem = false;
    for (int i = firstOverflowIndex; i < m_items.size(); ++i) {
        Item &item = m_items[i];
        if (!placedItem && item.separator) {
            item.widget->setParent(m_overflowPanel);
            item.widget->hide();
            continue;
        }
        const int widgetWidth = itemWidth(item);
        int widthWithSpacing = widgetWidth + (itemCountInRow > 0 ? ToolbarSpacing : 0);
        if (itemCountInRow > 0 && rowWidth + widthWithSpacing > availableWidth) {
            rowLayout = createRowLayout();
            itemCountInRow = 0;
            rowWidth = leftInset + rightInset;
            widthWithSpacing = widgetWidth;
        }
        item.widget->setParent(m_overflowPanel);
        item.widget->show();
        rowLayout->addWidget(item.widget);
        rowWidth += widthWithSpacing;
        ++itemCountInRow;
        placedItem = true;
    }
    m_overflowLayout->invalidate();
    const QMargins frameMargins = m_overflowPanel->contentsMargins();
    const int panelWidth = m_overflowLayout->sizeHint().width()
        + frameMargins.left() + frameMargins.right();
    m_overflowPanel->setFixedWidth(panelWidth);
    m_overflowPanel->adjustSize();
}

void PersistentToolBar::positionOverflowPanel()
{
    if (!m_overlayParent || !m_overflowPanel)
        return;
    m_overflowPanel->adjustSize();
    const QPoint belowToolbar = mapTo(m_overlayParent, QPoint(0, height()));
    const int panelLeft = belowToolbar.x() - overflowPanelLeftInset();
    const int maxX = std::max(0, m_overlayParent->width() - m_overflowPanel->width());
    const int x = std::clamp(panelLeft, 0, maxX);
    m_overflowPanel->move(x, belowToolbar.y());
}

void PersistentToolBar::updateExtensionButton()
{
    if (!m_extensionButton)
        return;
    const bool expanded = m_extensionButton->isChecked() && hasOverflow();
    m_extensionButton->setIcon(style()->standardIcon(expanded ? QStyle::SP_ArrowUp : QStyle::SP_ArrowDown));
    m_extensionButton->setIconSize(m_iconSize);
}
