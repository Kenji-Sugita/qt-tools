#include "AnkerFormLayout.h"

#include <QWidget>

#include <algorithm>
#include <optional>

namespace Anker {
namespace {

bool isHorizontal(Edge edge)
{
    return edge == Edge::Left || edge == Edge::Right;
}

int nearEdge(const QRect &rect, Edge edge)
{
    switch (edge) {
    case Edge::Left:
        return rect.left();
    case Edge::Right:
        return rect.right() + 1;
    case Edge::Top:
        return rect.top();
    case Edge::Bottom:
        return rect.bottom() + 1;
    }
    return 0;
}

int farWidgetEdge(const QRect &rect, Edge edge)
{
    switch (edge) {
    case Edge::Left:
        return rect.right() + 1;
    case Edge::Right:
        return rect.left();
    case Edge::Top:
        return rect.bottom() + 1;
    case Edge::Bottom:
        return rect.top();
    }
    return 0;
}

int oppositeFormEdge(const QRect &rect, Edge edge)
{
    switch (edge) {
    case Edge::Left:
        return rect.right() + 1;
    case Edge::Right:
        return rect.left();
    case Edge::Top:
        return rect.bottom() + 1;
    case Edge::Bottom:
        return rect.top();
    }
    return 0;
}

int inwardOffset(Edge edge, int value, int offset)
{
    return (edge == Edge::Right || edge == Edge::Bottom) ? value - offset : value + offset;
}

const FormAttachment &attachmentFor(const FormConstraints &constraints, Edge edge)
{
    switch (edge) {
    case Edge::Left:
        return constraints.left;
    case Edge::Right:
        return constraints.right;
    case Edge::Top:
        return constraints.top;
    case Edge::Bottom:
        return constraints.bottom;
    }
    return constraints.left;
}

std::optional<int> resolveAttachment(const FormAttachment &attachment,
                                     Edge edge,
                                     const QRect &contentsRect,
                                     const QList<QRect> &geometries,
                                     const QList<QRect> &selfGeometries,
                                     const QHash<QWidget *, int> &widgetIndexes,
                                     int fractionBase,
                                     int itemIndex,
                                     bool rubberPositioning,
                                     const QRect &selfReferenceRect,
                                     int spacing)
{
    const int offset = attachment.usesDefaultSpacing() ? spacing : attachment.offset();

    switch (attachment.type()) {
    case FormAttachment::Type::None:
        return std::nullopt;
    case FormAttachment::Type::Form:
        return inwardOffset(edge, nearEdge(contentsRect, edge), offset);
    case FormAttachment::Type::OppositeForm:
        return inwardOffset(edge, oppositeFormEdge(contentsRect, edge), offset);
    case FormAttachment::Type::Widget: {
        const auto index = widgetIndexes.constFind(attachment.target());
        if (!attachment.target() || index == widgetIndexes.constEnd())
            return std::nullopt;
        return inwardOffset(edge, farWidgetEdge(geometries.at(index.value()), edge), offset);
    }
    case FormAttachment::Type::OppositeWidget: {
        const auto index = widgetIndexes.constFind(attachment.target());
        if (!attachment.target() || index == widgetIndexes.constEnd())
            return std::nullopt;
        return inwardOffset(edge, nearEdge(geometries.at(index.value()), edge), offset);
    }
    case FormAttachment::Type::Position: {
        const int span = isHorizontal(edge) ? contentsRect.width() : contentsRect.height();
        const int origin = isHorizontal(edge) ? contentsRect.left() : contentsRect.top();
        const int position = origin + span * attachment.position() / std::max(1, fractionBase);
        return inwardOffset(edge, position, offset);
    }
    case FormAttachment::Type::Self: {
        if (itemIndex < 0 || itemIndex >= selfGeometries.size())
            return std::nullopt;

        const QRect self = selfGeometries.at(itemIndex);
        int value = nearEdge(self, edge);
        if (rubberPositioning && selfReferenceRect.isValid()) {
            const int oldSpan = std::max(1, isHorizontal(edge) ? selfReferenceRect.width() : selfReferenceRect.height());
            const int newSpan = isHorizontal(edge) ? contentsRect.width() : contentsRect.height();
            const int oldOrigin = isHorizontal(edge) ? selfReferenceRect.left() : selfReferenceRect.top();
            const int newOrigin = isHorizontal(edge) ? contentsRect.left() : contentsRect.top();
            value = newOrigin + (value - oldOrigin) * newSpan / oldSpan;
        }
        return inwardOffset(edge, value, offset);
    }
    }
    return std::nullopt;
}

QRect itemInitialGeometry(QLayoutItem *item, const QRect &contents)
{
    const QSize hint = item->sizeHint().expandedTo(item->minimumSize());
    QRect geometry = item->geometry();
    if (!geometry.isValid() || geometry.isNull())
        geometry = QRect(contents.topLeft(), hint);
    if (geometry.width() <= 0)
        geometry.setWidth(hint.width());
    if (geometry.height() <= 0)
        geometry.setHeight(hint.height());
    return geometry;
}

} // namespace

FormLayout::FormLayout(QWidget *parent)
    : QLayout(parent)
{
}

FormLayout::~FormLayout()
{
    QLayoutItem *item = nullptr;
    while ((item = takeAt(0)) != nullptr)
        delete item;
}

void FormLayout::addItem(QLayoutItem *item)
{
    m_entries.append(Entry {item, {}});
    invalidate();
}

void FormLayout::addWidget(QWidget *widget, const FormConstraints &constraints)
{
    addChildWidget(widget);
    m_entries.append({new QWidgetItem(widget), constraints});
    invalidate();
}

int FormLayout::count() const
{
    return m_entries.size();
}

QLayoutItem *FormLayout::itemAt(int index) const
{
    return index >= 0 && index < m_entries.size() ? m_entries.at(index).item : nullptr;
}

QLayoutItem *FormLayout::takeAt(int index)
{
    if (index < 0 || index >= m_entries.size())
        return nullptr;

    const Entry entry = m_entries.takeAt(index);
    invalidate();
    return entry.item;
}

QSize FormLayout::sizeHint() const
{
    return expandedItemSize(&QLayoutItem::sizeHint);
}

QSize FormLayout::minimumSize() const
{
    return expandedItemSize(&QLayoutItem::minimumSize);
}

void FormLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);

    const QRect contents = contentsRect();
    QList<QRect> geometries;
    QList<QRect> selfGeometries;
    QHash<QWidget *, int> widgetIndexes;
    geometries.reserve(m_entries.size());
    selfGeometries.reserve(m_entries.size());

    if (!m_selfReferenceRect.isValid())
        m_selfReferenceRect = contents;

    for (int i = 0; i < m_entries.size(); ++i) {
        QLayoutItem *item = m_entries.at(i).item;
        if (!m_entries[i].hasSelfGeometry) {
            m_entries[i].selfGeometry = itemInitialGeometry(item, contents);
            m_entries[i].hasSelfGeometry = true;
        }

        geometries.append(m_entries.at(i).selfGeometry);
        selfGeometries.append(m_entries.at(i).selfGeometry);
        if (QWidget *widget = item->widget())
            widgetIndexes.insert(widget, i);
    }

    const int passCount = std::max(2, static_cast<int>(m_entries.size()) + 1);
    for (int pass = 0; pass < passCount; ++pass) {
        for (int i = 0; i < m_entries.size(); ++i) {
            QLayoutItem *item = m_entries.at(i).item;
            const FormConstraints &constraints = m_entries.at(i).constraints;
            const QSize hint = item->sizeHint().expandedTo(item->minimumSize());

            const std::optional<int> left =
                resolveAttachment(attachmentFor(constraints, Edge::Left), Edge::Left, contents, geometries, selfGeometries, widgetIndexes, m_fractionBase, i, m_rubberPositioning, m_selfReferenceRect, effectiveSpacing(Edge::Left));
            const std::optional<int> right =
                resolveAttachment(attachmentFor(constraints, Edge::Right), Edge::Right, contents, geometries, selfGeometries, widgetIndexes, m_fractionBase, i, m_rubberPositioning, m_selfReferenceRect, effectiveSpacing(Edge::Right));
            const std::optional<int> top =
                resolveAttachment(attachmentFor(constraints, Edge::Top), Edge::Top, contents, geometries, selfGeometries, widgetIndexes, m_fractionBase, i, m_rubberPositioning, m_selfReferenceRect, effectiveSpacing(Edge::Top));
            const std::optional<int> bottom =
                resolveAttachment(attachmentFor(constraints, Edge::Bottom), Edge::Bottom, contents, geometries, selfGeometries, widgetIndexes, m_fractionBase, i, m_rubberPositioning, m_selfReferenceRect, effectiveSpacing(Edge::Bottom));

            int width = hint.width();
            int height = hint.height();
            int x = geometries.at(i).x();
            int y = geometries.at(i).y();

            if (left && right) {
                x = *left;
                width = std::max(item->minimumSize().width(), *right - *left);
            } else if (left) {
                x = *left;
            } else if (right) {
                x = *right - width;
            }

            if (top && bottom) {
                y = *top;
                height = std::max(item->minimumSize().height(), *bottom - *top);
            } else if (top) {
                y = *top;
            } else if (bottom) {
                y = *bottom - height;
            }

            geometries[i] = QRect(x, y, width, height);
        }
    }

    for (int i = 0; i < m_entries.size(); ++i)
        m_entries.at(i).item->setGeometry(geometries.at(i));
}

int FormLayout::fractionBase() const
{
    return m_fractionBase;
}

void FormLayout::setFractionBase(int fractionBase)
{
    m_fractionBase = std::max(1, fractionBase);
    invalidate();
}

bool FormLayout::rubberPositioning() const
{
    return m_rubberPositioning;
}

void FormLayout::setRubberPositioning(bool enabled)
{
    m_rubberPositioning = enabled;
    invalidate();
}

int FormLayout::horizontalSpacing() const
{
    return std::max(0, m_horizontalSpacing >= 0 ? m_horizontalSpacing : spacing());
}

void FormLayout::setHorizontalSpacing(int spacing)
{
    m_horizontalSpacing = spacing;
    invalidate();
}

int FormLayout::verticalSpacing() const
{
    return std::max(0, m_verticalSpacing >= 0 ? m_verticalSpacing : spacing());
}

void FormLayout::setVerticalSpacing(int spacing)
{
    m_verticalSpacing = spacing;
    invalidate();
}

FormConstraints FormLayout::constraintsFor(QWidget *widget) const
{
    const int index = indexOf(widget);
    return index >= 0 ? m_entries.at(index).constraints : FormConstraints {};
}

void FormLayout::setConstraints(QWidget *widget, const FormConstraints &constraints)
{
    const int index = indexOf(widget);
    if (index < 0)
        return;

    m_entries[index].constraints = constraints;
    if (m_entries[index].item) {
        m_entries[index].selfGeometry = m_entries[index].item->geometry();
        m_entries[index].hasSelfGeometry = m_entries[index].selfGeometry.isValid();
    }
    invalidate();
}

int FormLayout::indexOf(QWidget *widget) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).item->widget() == widget)
            return i;
    }
    return -1;
}

QSize FormLayout::expandedItemSize(QSize (QLayoutItem::*sizeFunction)() const) const
{
    QSize size;
    for (const Entry &entry : m_entries)
        size = size.expandedTo((entry.item->*sizeFunction)());

    QMargins margins = contentsMargins();
    size.rwidth() += margins.left() + margins.right();
    size.rheight() += margins.top() + margins.bottom();
    return size;
}

int FormLayout::effectiveSpacing(Edge edge) const
{
    return isHorizontal(edge) ? horizontalSpacing() : verticalSpacing();
}

} // namespace Anker
