#include "gizmo.h"
#include <QApplication>
#include <QLayout>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget top;
    const auto topLayout = new QVBoxLayout(&top);
    topLayout->addWidget(new Gizmo(Qt::blue, Qt::green, Qt::Horizontal));
    topLayout->addWidget(new Gizmo(Qt::yellow, Qt::black, Qt::Vertical));
    topLayout->addWidget(new Gizmo(Qt::white, Qt::green, Qt::Horizontal));

    top.show();

    return app.exec();
}
