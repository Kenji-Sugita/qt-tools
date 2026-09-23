#include "server.h"
#include "connectionwindow.h"
#include <QApplication>
#include <QPushButton>
#include <QLocalSocket>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    if (argc == 2 && qstrcmp(argv[1], "-client") == 0) {
        // Run this as a client
        const auto socket = new QLocalSocket;  // Potential memory leak.
        socket->connectToServer(SERVER_NAME);
        ConnectionWindow* const client = new ConnectionWindow(socket, 0);
        client->show();
    } else {
        // Run this as a server.
        new Server;  // Potential memory leak.

        // Provide a button for quitting the server process; especially useful on Windows.
        const auto quitButton = new QPushButton("Quit server", 0);  // Potential memory leak.
        quitButton->setMinimumSize(quitButton->sizeHint());
        quitButton->show();
        QObject::connect(quitButton, &QPushButton::clicked, qApp, &QApplication::quit);
    }

    // If we didn't have a button then we would need to tell QApplication:
    // "don't quit after closing the first window":
    //qApp->setQuitOnLastWindowClosed(false);

    return app.exec();
}
