#pragma once

#include <QtCore/qglobal.h>

#if defined(OBJECT_SELECTOR_LIBRARY)
#  define OBJECT_SELECTOR_EXPORT Q_DECL_EXPORT
#else
#  define OBJECT_SELECTOR_EXPORT Q_DECL_IMPORT
#endif
