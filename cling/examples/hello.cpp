#include <QtWidgets/QtWidgets>

void hello()
{
    int argc = 1;
    char appName[] = "qtcling";
    char* argv[] = {appName, nullptr};

    QApplication app(argc, argv);

    QWidget window{};
    auto topLayout = new QVBoxLayout(&window);

    auto button = new QPushButton{"Quit"};
    topLayout->addWidget(button);
    QObject::connect(button, &QPushButton::clicked, &app, &QApplication::quit);

    window.show();

    app.exec();
}
