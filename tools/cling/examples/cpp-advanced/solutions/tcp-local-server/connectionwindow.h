#ifndef CONNECTIONWINDOW_H
#define CONNECTIONWINDOW_H

#include <QWidget>
class QLineEdit;
class QTextEdit;
class QLocalSocket;

class ConnectionWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWindow(QLocalSocket* socket, QWidget* parent = nullptr);

private slots:
    void slotRead();
    void slotSendLine();

private:
    QLocalSocket* _socket;
    QTextEdit* _in;
    QLineEdit* _out;
};
#endif
