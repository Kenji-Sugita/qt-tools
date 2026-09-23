#pragma once

#include <QObject>

#include <functional>

class BoardModel;
class QByteArray;
class CanvasWidget;
class QIODevice;
class QJsonObject;
class QMcpServer;

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
    bool registerTools();
    QJsonObject stateTool(const QJsonObject &arguments, QString *errorMessage) const;
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
    QMcpServer *m_server = nullptr;
    bool m_toolsRegistered = false;
};
