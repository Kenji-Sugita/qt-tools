#include <QtWidgets>
#include <QtNetwork>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QList<const QMetaObject*> staticMetaObjectTable = {
        &QObject::staticMetaObject,
        &QTimer::staticMetaObject,
        &QAbstractItemModel::staticMetaObject,
        &QPushButton::staticMetaObject,
        &QTcpSocket::staticMetaObject,
    };

    QTextStream tout(stdout);
    const QString indent = QString(" ").repeated(4);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    for (const auto& metaObject : std::as_const(staticMetaObjectTable)) {
#else
    for (const auto& metaObject : qAsConst(staticMetaObjectTable)) {
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        tout << metaObject->className() << ":" << Qt::endl;
#else
        tout << metaObject->className() << ":" << endl;
#endif
        for (int methoCounter = 0; methoCounter < metaObject->methodCount(); methoCounter++) {
            const QMetaMethod& method = metaObject->method(methoCounter);
            switch (method.access()) {
            case QMetaMethod::Private:
                tout << indent << "private ";
                break;
            case QMetaMethod::Protected:
                tout << indent << "protected ";
                break;
            case QMetaMethod::Public:
                tout << indent << "public ";
                break;
            }
            tout << method.typeName() << " " << method.name() << "(";
            for (int parameterCount = 0; parameterCount < method.parameterTypes().count(); parameterCount++) {
                tout << method.parameterTypes()[parameterCount];
                if (!method.parameterNames().at(parameterCount).isEmpty()) {
                    tout << " " << method.parameterNames()[parameterCount];
                }
                if (parameterCount < (method.parameterTypes().count() - 1)) {
                    tout << ", ";
                }
            }
            tout << ")";
            switch (method.methodType()) {
            case QMetaMethod::Method:
                tout << " method";
                break;
            case QMetaMethod::Signal:
                tout << " signal";
                break;
            case QMetaMethod::Slot:
                tout << " slot";
                break;
            case QMetaMethod::Constructor:
                break;
            }
            if (methoCounter < metaObject->methodOffset()) {
                tout << " inherited";
            }
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            tout << Qt::endl;
#else
            tout << endl;
#endif
        }
    }
}
