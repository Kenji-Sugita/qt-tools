#include "server.h"
#include "connectionwindow.h"

Server::Server(QObject* parent)
    : QLocalServer(parent)
{
    // Start listening on port 4242
    if (!listen(SERVER_NAME)) {
	if (serverError() == QAbstractSocket::AddressInUseError) {
	    removeServer(SERVER_NAME);
	    if (!listen(SERVER_NAME)) {
		qDebug() << errorString();
	    }
	}
    }
    connect(this, &Server::newConnection, this, &Server::slotConnection);
}

void Server::slotConnection()
{
    // Handle the incomming connection
    while (hasPendingConnections()) {
        QLocalSocket* const socket = nextPendingConnection();
        const auto connection = new ConnectionWindow(socket, 0);
        connection->setAttribute(Qt::WA_DeleteOnClose);
        connection->show();
    }
}
