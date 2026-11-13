#pragma once

#include <QObject>
#include <QString>

class QTcpSocket;

class SseSession : public QObject
{
    Q_OBJECT

public:
    SseSession(QString id, QTcpSocket *socket, QObject *parent = nullptr);

    QString id() const;
    void start(const QString &messageEndpoint);
    void sendMessage(const QString &json);

signals:
    void closed(const QString &id);

private:
    QString m_id;
    QTcpSocket *m_socket = nullptr;
};
