#include "LaserPointerWidget.h"

#include <QApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Laser Pointer"));
    QApplication::setOrganizationName(QStringLiteral("QtTools"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/laser-pointer.png")));
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QApplication::setQuitOnLastWindowClosed(false);
#endif

    LaserPointerWidget pointer;

    const QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    pointer.move(screenGeometry.center() - pointer.rect().center());
    pointer.show();

    return app.exec();
}
