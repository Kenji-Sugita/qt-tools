#pragma once

#include "AnkerFormConstraints.h"

#include <QHash>
#include <QLayout>
#include <QList>

namespace Anker {

class FormLayout : public QLayout
{
public:
    explicit FormLayout(QWidget *parent = nullptr);
    ~FormLayout() override;

    void addItem(QLayoutItem *item) override;
    void addWidget(QWidget *widget, const FormConstraints &constraints);

    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;

    QSize sizeHint() const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;

    int fractionBase() const;
    void setFractionBase(int fractionBase);

    bool rubberPositioning() const;
    void setRubberPositioning(bool enabled);

    int horizontalSpacing() const;
    void setHorizontalSpacing(int spacing);

    int verticalSpacing() const;
    void setVerticalSpacing(int spacing);

    FormConstraints constraintsFor(QWidget *widget) const;
    void setConstraints(QWidget *widget, const FormConstraints &constraints);

private:
    struct Entry
    {
        QLayoutItem *item = nullptr;
        FormConstraints constraints;
        QRect selfGeometry;
        bool hasSelfGeometry = false;
    };

    int indexOf(QWidget *widget) const;
    QSize expandedItemSize(QSize (QLayoutItem::*sizeFunction)() const) const;
    int effectiveSpacing(Edge edge) const;

    QList<Entry> m_entries;
    int m_fractionBase = 100;
    bool m_rubberPositioning = false;
    int m_horizontalSpacing = -1;
    int m_verticalSpacing = -1;
    QRect m_selfReferenceRect;
};

} // namespace Anker
