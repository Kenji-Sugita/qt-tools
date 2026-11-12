#pragma once

#include <QPointer>

#include <limits>

class QWidget;

namespace Anker {

enum class Edge
{
    Left,
    Right,
    Top,
    Bottom,
};

class FormAttachment
{
public:
    static constexpr int DefaultSpacing = std::numeric_limits<int>::min();

    enum class Type
    {
        None,
        Form,
        OppositeForm,
        Widget,
        OppositeWidget,
        Position,
        Self,
    };

    static FormAttachment none();
    static FormAttachment form(int offset = 0);
    static FormAttachment oppositeForm(int offset = 0);
    static FormAttachment widget(QWidget *target, int offset = DefaultSpacing);
    static FormAttachment oppositeWidget(QWidget *target, int offset = 0);
    static FormAttachment position(int position, int offset = 0);
    static FormAttachment self(int offset = 0);

    Type type() const;
    QWidget *target() const;
    int position() const;
    int offset() const;
    bool usesDefaultSpacing() const;

private:
    FormAttachment(Type type, QWidget *target, int position, int offset);

    Type m_type = Type::None;
    QPointer<QWidget> m_target;
    int m_position = 0;
    int m_offset = 0;
};

FormAttachment attachNone();
FormAttachment attachForm(int offset = 0);
FormAttachment attachOppositeForm(int offset = 0);
FormAttachment attachWidget(QWidget *target, int offset = FormAttachment::DefaultSpacing);
FormAttachment attachOppositeWidget(QWidget *target, int offset = 0);
FormAttachment attachPosition(int position, int offset = 0);
FormAttachment attachSelf(int offset = 0);

} // namespace Anker
