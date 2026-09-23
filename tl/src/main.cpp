// tl のエントリポイント。起動処理だけを持つ薄い main。

#include "tlapplication.h"

#include <QCoreApplication>

namespace {

constexpr auto kVersion = "0.1";

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("tl"));
    QCoreApplication::setApplicationVersion(QString::fromLatin1(kVersion));

    TlApplication tlApplication;
    return tlApplication.run(app);
}
