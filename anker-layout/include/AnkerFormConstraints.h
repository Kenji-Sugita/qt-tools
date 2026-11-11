#pragma once

#include "AnkerFormAttachment.h"

namespace Anker {

struct FormConstraints
{
    FormAttachment left = attachNone();
    FormAttachment right = attachNone();
    FormAttachment top = attachNone();
    FormAttachment bottom = attachNone();
};

} // namespace Anker
