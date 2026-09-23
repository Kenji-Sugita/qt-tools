#include "server.h"
#include "connectionwindow.h"

Server::Server(QObject* parent)
    : QTcpServer(parent)
{
    // TODO: Start listening on port 4242
    connect(this, &Server::newConnection, this, &Server::slotConnection);
}

void Server::slotConnection()
{
    // TODO: handle the incomming connection
}
