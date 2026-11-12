#pragma once

#if defined(WIDGET_DESIGNER_PLUGIN)
    #include <QtUiPlugin/QDesignerExportWidget>
    #define WIDGET_EXPORT QDESIGNER_WIDGET_EXPORT
#else
    #define WIDGET_EXPORT
#endif
