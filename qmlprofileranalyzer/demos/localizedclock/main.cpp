#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [](const QUrl &) { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(QUrl::fromLocalFile(QStringLiteral(LOCALIZEDCLOCKDEMO_QML_PATH)));

    // Keep the demo deterministic enough for profiling runs while allowing
    // multiple timer-driven updates to occur.
    QTimer::singleShot(15000, &app, &QCoreApplication::quit);

    return app.exec();
}
