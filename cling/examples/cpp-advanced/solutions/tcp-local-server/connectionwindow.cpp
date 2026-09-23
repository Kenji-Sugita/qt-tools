#include "connectionwindow.h"
#include <QLayout>
#include <QLocalSocket>
#include <QTextEdit>
#include <QLineEdit>

ConnectionWindow::ConnectionWindow(QLocalSocket* socket, QWidget* parent)
    : QWidget(parent), _socket(socket)
{
    _in = new QTextEdit;
    _in->setReadOnly(true);

    _out = new QLineEdit;

    connect(_out, &QLineEdit::returnPressed, this, &ConnectionWindow::slotSendLine);
    // TODO: Set up signal/slot connection for the socket.

    // TODO: Close the window when the client is disconnected
    //       Tip: QWidget::close() is a slot.
    connect(_socket, &QLocalSocket::readyRead, this, &ConnectionWindow::slotRead);
    connect(_socket, &QLocalSocket::disconnected, this, &ConnectionWindow::close);

    const auto layout = new QVBoxLayout(this);
    layout->addWidget(_in);
    layout->addWidget(_out);

    QTextStream stream(_socket);
    stream << "Hello World\n";
}

void ConnectionWindow::slotRead()
{
    // TODO: Implement
    while (_socket->canReadLine()) {
        QString line = _socket->readLine();
        if (line.endsWith("\r\n")) {
            line.chop(2);
        } else if (line.endsWith("\n")) {
            line.chop(1);
        }
        _in->append(line);
    }
}

void ConnectionWindow::slotSendLine()
{
    // TODO: Implement
    QTextStream stream(_socket);
    const QString text = _out->text();
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    stream << text << Qt::endl;
#else
    stream << text << endl;
#endif
    _out->clear();
}
