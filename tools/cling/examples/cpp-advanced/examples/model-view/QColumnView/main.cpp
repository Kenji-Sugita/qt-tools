#include <QApplication>
#include <QColumnView>
#include <QFileSystemModel>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QColumnView view;
    QFileSystemModel model;
    model.setRootPath(QDir::root().path());
    view.setModel(&model);
    view.setMinimumSize(800, 600);
    view.show();

    return app.exec();
}
