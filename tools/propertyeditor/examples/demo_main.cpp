#include "demo_widget_selector.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window{}; 
    window.setWindowTitle("Property Editor Demo");
    DemoWidgetSelector::setupDemoWindow(&window);
    window.show();

    return app.exec();
}
