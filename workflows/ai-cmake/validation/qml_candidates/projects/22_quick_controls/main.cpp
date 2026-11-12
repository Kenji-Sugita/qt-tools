#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.loadFromModule("Qml22QuickControls", "Main");
    return engine.rootObjects().isEmpty() ? 1 : 0;
}
