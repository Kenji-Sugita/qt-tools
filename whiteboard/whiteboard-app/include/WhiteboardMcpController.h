#pragma once

#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QString>
#include <QVector>

#include <functional>

class BoardModel;
class QByteArray;
class CanvasWidget;
class QIODevice;

class WhiteboardMcpController : public QObject {
public:
    using ChangeCallback = std::function<void()>;
    using PageNavigationCallback = std::function<bool(const QString &)>;
    using ApprovalCallback = std::function<bool(const QString &, const QJsonObject &, QString *)>;

    WhiteboardMcpController(BoardModel *model,
                            CanvasWidget *canvas,
                            ChangeCallback changeCallback,
                            PageNavigationCallback pageNavigationCallback,
                            ApprovalCallback approvalCallback,
                            QObject *parent = nullptr);
    ~WhiteboardMcpController() override;

    void setInputDevice(QIODevice *device);
    void setOutputDevice(QIODevice *device);
    bool start();
    bool startMessageTransport();
    bool processMessage(const QByteArray &message, QJsonObject *response, bool *hasResponse);
    QString negotiatedProtocolVersion() const;
    void stop();

private:
    using ToolCallback = std::function<QJsonObject(const QJsonObject &, QString *)>;

    struct ToolDefinition {
        QString name;
        QString description;
        QJsonObject inputSchema;
        bool requiresApproval = false;
        ToolCallback callback;
    };

    bool registerTool(const QString &name,
                      const QString &description,
                      ToolCallback callback,
                      const QJsonObject &inputSchema,
                      bool requiresApproval = false);
    bool registerTools();
    bool processRequest(const QJsonObject &request, QJsonObject *response, bool *hasResponse);
    QJsonObject makeResultResponse(const QJsonValue &id, const QJsonObject &result) const;
    QJsonObject makeErrorResponse(const QJsonValue &id, int code, const QString &message) const;
    QJsonObject initializeRequest(const QJsonObject &params, QString *errorMessage);
    QJsonObject toolsListRequest() const;
    QJsonObject toolsCallRequest(const QJsonObject &params, QString *errorMessage);
    void processInputDeviceMessages();
    QJsonObject stateTool(const QJsonObject &arguments, QString *errorMessage) const;
    QJsonObject listElementsTool(const QJsonObject &arguments, QString *errorMessage) const;
    QJsonObject updateElementsTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject deleteElementsTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject applyElementChangesTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject listPagesTool(const QJsonObject &arguments, QString *errorMessage) const;
    QJsonObject navigatePageTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject addPageTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject deletePageTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject lockPageTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject applyDiagramTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject renderPageTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject exportImageTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject saveImageTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject undoTool(const QJsonObject &arguments, QString *errorMessage);
    QJsonObject redoTool(const QJsonObject &arguments, QString *errorMessage);

    BoardModel *m_model = nullptr;
    CanvasWidget *m_canvas = nullptr;
    ChangeCallback m_changeCallback;
    PageNavigationCallback m_pageNavigationCallback;
    ApprovalCallback m_approvalCallback;
    QIODevice *m_inputDevice = nullptr;
    QIODevice *m_outputDevice = nullptr;
    QByteArray m_inputBuffer;
    QVector<ToolDefinition> m_tools;
    QHash<QString, int> m_toolIndexes;
    QString m_negotiatedProtocolVersion;
    bool m_toolsRegistered = false;
    bool m_started = false;
};
