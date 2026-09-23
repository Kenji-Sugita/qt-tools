#include "connectionwindow.h"
#include <QTcpSocket>
#include <QLayout>
#include <QTextEdit>
#include <QLineEdit>

ConnectionWindow::ConnectionWindow(QTcpSocket* socket, QWidget* parent)
    : QWidget(parent), _socket(socket)
{
    _in = new QTextEdit;
    _in->setReadOnly(true);

    _out = new QLineEdit;

    connect(_out, &QLineEdit::returnPressed, this, &ConnectionWindow::slotSendLine);
    // TODO: Set up signal/slot connection for the socket.

    // TODO: Close the window when the client is disconnected
    //       Tip: QWidget::close() is a slot.

    const auto layout = new QVBoxLayout(this);
    layout->addWidget(_in);
    layout->addWidget(_out);
}

void ConnectionWindow::slotRead()
{
    // TODO: Implement
}

void ConnectionWindow::slotSendLine()
{
    // TODO: Implement
}
