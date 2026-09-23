#pragma once

#include <QtCore/qglobal.h>

#if defined(PROPERTY_EDITOR_LIBRARY)
#  define PROPERTY_EDITOR_EXPORT Q_DECL_EXPORT
#else
#  define PROPERTY_EDITOR_EXPORT Q_DECL_IMPORT
#endif
