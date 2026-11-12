#include "gizmo.h"
#include <QApplication>
#include <QLayout>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Gizmo* gizmo1 = new Gizmo(Qt::blue, Qt::green, Qt::Horizontal);
    Gizmo* gizmo2 = new Gizmo(Qt::yellow, Qt::black, Qt::Vertical);
    Gizmo* gizmo3 = new Gizmo(Qt::white, Qt::green, Qt::Horizontal);

    QWidget top;
    const auto topLayout = new QVBoxLayout(&top);
    topLayout->addWidget(gizmo1);
    topLayout->addWidget(gizmo2);
    topLayout->addWidget(gizmo3);

    top.show();

    return app.exec();
}
