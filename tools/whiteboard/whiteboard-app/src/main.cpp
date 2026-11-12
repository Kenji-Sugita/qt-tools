#include "MainWindow.h"

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>

#ifndef WHITEBOARD_APP_VERSION
#define WHITEBOARD_APP_VERSION "0.0.0"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationVersion(QStringLiteral(WHITEBOARD_APP_VERSION));
    app.setWindowIcon(QIcon(QStringLiteral(":/assets/appicon/whiteboard-app-icon.png")));

    QTranslator appTranslator;
    if (QLocale::system().language() == QLocale::Japanese
        && appTranslator.load(QLocale::system(), QStringLiteral("whiteboard"), QStringLiteral("_"), QStringLiteral(":/i18n"))) {
        app.installTranslator(&appTranslator);
    }

    MainWindow window;
    const bool stdioMcp = app.arguments().contains(QStringLiteral("--mcp"));
    if (stdioMcp) {
        if (!window.startMcpServer())
            return 1;
    } else if (!window.startMcpHttpServer()) {
        window.show();
        QMessageBox::warning(
            &window,
            QObject::tr("MCP Server"),
            QObject::tr("The local MCP HTTP server could not start: %1")
                .arg(window.mcpHttpErrorString()));
    }
    window.show();
    return app.exec();
}
