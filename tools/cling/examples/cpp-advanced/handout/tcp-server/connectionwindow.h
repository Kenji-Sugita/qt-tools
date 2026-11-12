#ifndef CONNECTIONWINDOW_H
#define CONNECTIONWINDOW_H

#include <QWidget>
class QLineEdit;
class QTextEdit;
class QTcpSocket;

class ConnectionWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWindow(QTcpSocket* socket, QWidget* parent = nullptr);

private slots:
    void slotRead();
    void slotSendLine();

private:
    QTcpSocket* _socket;
    QTextEdit* _in;
    QLineEdit* _out;
};
#endif
