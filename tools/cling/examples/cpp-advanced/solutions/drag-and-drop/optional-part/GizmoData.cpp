#include "GizmoData.h"
#include "gizmo.h"
#include <QStringList>
#include <QIODevice>
#include <QDebug>

GizmoData::GizmoData(Gizmo* gizmo)
    : QMimeData(), gizmo(gizmo)
{
}

QStringList GizmoData::formats() const
{
    return { "text/plain", "x-gizmo/x-drag" };
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QVariant GizmoData::retrieveData(const QString& mimeType, QMetaType /*type*/) const
#else
QVariant GizmoData::retrieveData(const QString& mimeType, QVariant::Type /*type*/) const
#endif
{
    if (mimeType == QLatin1String("text/plain")) {
        return "Hello World";
    } else if (mimeType == QLatin1String("text/html")) {
        return "Hello World";
    } else if (mimeType == QLatin1String("x-gizmo/x-drag")) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << gizmo->color1 << gizmo->color2 << static_cast<quint32>(gizmo->orientation);
        return data;
    }
#ifdef Q_OS_MAC
    else if (mimeType == QLatin1String("application/x-qt-mime-type-name")) {
        return QVariant();
    }
    else if (mimeType == QLatin1String("text/uri-list")) {  // Workaround for Qt 5.15.2
        return QVariant();
    }
#endif
    else {
        qWarning() << mimeType << "A type we did not claim we could support, this should not happen!";
        return QVariant();
    }
}

bool GizmoData::decode(Gizmo* destination, const QMimeData* mime)
{
    if (mime->hasFormat("x-gizmo/x-drag")) {
        const QByteArray data = mime->data("x-gizmo/x-drag");
        QDataStream stream(data);
        Q_ASSERT(sizeof(Qt::Orientation) == sizeof(int));
        stream >> destination->color1 >> destination->color2 >> reinterpret_cast<int&>(destination->orientation);
        return true;
    }
    return false;
}
