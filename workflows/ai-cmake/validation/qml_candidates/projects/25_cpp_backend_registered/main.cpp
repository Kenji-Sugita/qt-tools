#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qqml.h>
#include "backend.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qmlRegisterType<Backend>("Qml25CppBackend", 1, 0, "Backend");
    QQmlApplicationEngine engine;
    engine.loadFromModule("Qml25CppBackend", "Main");
    return engine.rootObjects().isEmpty() ? 1 : 0;
}
