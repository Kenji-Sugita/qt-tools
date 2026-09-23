#ifndef SERVER_H
#define SERVER_H

#include <QLocalServer>
#define SERVER_NAME "Chat"

class Server : public QLocalServer
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);

protected slots:
    void slotConnection();
};
#endif
