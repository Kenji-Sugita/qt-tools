#pragma once

#include <QByteArray>
#include <QEventLoop>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

class SseMcpClient : public QObject
{
    Q_OBJECT

public:
    explicit SseMcpClient(QUrl url, QObject *parent = nullptr);

    bool connectToServer(int timeoutMs, QString *error);
    QJsonObject request(const QString &method,
                        const QJsonObject &params,
                        int timeoutMs,
                        QString *error);
    QJsonObject callTool(const QString &name,
                         const QJsonObject &arguments,
                         int timeoutMs,
                         QString *error);

    QUrl url() const;
    bool isInitialized() const;

signals:
    void endpointReady();

private:
    void readSse();
    void dispatchSseEvent();
    void postJson(const QJsonObject &payload);
    void notify(const QString &method);

    QNetworkAccessManager m_network;
    QUrl m_sseUrl;
    QUrl m_postUrl;
    QNetworkReply *m_sseReply = nullptr;
    QByteArray m_sseBuffer;
    QString m_eventName;
    QByteArray m_eventData;
    QString m_lastError;
    int m_nextId = 0;
    bool m_initialized = false;
    QMap<int, QJsonObject> m_responses;
    QMap<int, QEventLoop *> m_waitingLoops;
};
