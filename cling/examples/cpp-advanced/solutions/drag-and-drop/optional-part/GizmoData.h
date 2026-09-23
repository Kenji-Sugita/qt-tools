#ifndef GIZMODATA_H
#define GIZMODATA_H

#include <QMimeData>

class Gizmo;

class GizmoData : public QMimeData
{
    Q_OBJECT

public:
    GizmoData(Gizmo* gizmo);
    virtual QStringList formats() const;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual QVariant retrieveData(const QString& mimeType, QMetaType type) const;
#else
    virtual QVariant retrieveData(const QString& mimeType, QVariant::Type type) const;
#endif
    static bool decode(Gizmo* destination, const QMimeData* data);

private:
    Gizmo* gizmo;
};

#endif /* GIZMODATA_H */

