#include "AnkerFormAttachment.h"

#include <QWidget>

namespace Anker {

FormAttachment::FormAttachment(Type type, QWidget *target, int position, int offset)
    : m_type(type)
    , m_target(target)
    , m_position(position)
    , m_offset(offset)
{
}

FormAttachment FormAttachment::none()
{
    return FormAttachment(Type::None, nullptr, 0, 0);
}

FormAttachment FormAttachment::form(int offset)
{
    return FormAttachment(Type::Form, nullptr, 0, offset);
}

FormAttachment FormAttachment::oppositeForm(int offset)
{
    return FormAttachment(Type::OppositeForm, nullptr, 0, offset);
}

FormAttachment FormAttachment::widget(QWidget *target, int offset)
{
    return FormAttachment(Type::Widget, target, 0, offset);
}

FormAttachment FormAttachment::oppositeWidget(QWidget *target, int offset)
{
    return FormAttachment(Type::OppositeWidget, target, 0, offset);
}

FormAttachment FormAttachment::position(int position, int offset)
{
    return FormAttachment(Type::Position, nullptr, position, offset);
}

FormAttachment FormAttachment::self(int offset)
{
    return FormAttachment(Type::Self, nullptr, 0, offset);
}

FormAttachment::Type FormAttachment::type() const
{
    return m_type;
}

QWidget *FormAttachment::target() const
{
    return m_target;
}

int FormAttachment::position() const
{
    return m_position;
}

int FormAttachment::offset() const
{
    return m_offset;
}

bool FormAttachment::usesDefaultSpacing() const
{
    return m_offset == DefaultSpacing;
}

FormAttachment attachNone()
{
    return FormAttachment::none();
}

FormAttachment attachForm(int offset)
{
    return FormAttachment::form(offset);
}

FormAttachment attachOppositeForm(int offset)
{
    return FormAttachment::oppositeForm(offset);
}

FormAttachment attachWidget(QWidget *target, int offset)
{
    return FormAttachment::widget(target, offset);
}

FormAttachment attachOppositeWidget(QWidget *target, int offset)
{
    return FormAttachment::oppositeWidget(target, offset);
}

FormAttachment attachPosition(int position, int offset)
{
    return FormAttachment::position(position, offset);
}

FormAttachment attachSelf(int offset)
{
    return FormAttachment::self(offset);
}

} // namespace Anker
