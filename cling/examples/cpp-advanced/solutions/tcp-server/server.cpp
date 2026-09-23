#include "server.h"
#include "connectionwindow.h"

Server::Server(QObject* parent)
    : QTcpServer(parent)
{
    listen(QHostAddress::LocalHost, 4242);
    connect(this, &Server::newConnection, this, &Server::slotConnection);
}

void Server::slotConnection()
{
    while (hasPendingConnections()) {
        QTcpSocket* const socket = nextPendingConnection();
        ConnectionWindow* const connection = new ConnectionWindow(socket, 0);
        connection->setAttribute(Qt::WA_DeleteOnClose);
        connection->show();
    }
}
