#ifndef SIMPLEIMAGEPLUGIN_H
#define SIMPLEIMAGEPLUGIN_H

#include <QImageIOPlugin>

class SimpleImagePlugin : public QImageIOPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "sif.json")

public:
    virtual Capabilities capabilities(QIODevice* device, const QByteArray& format) const override;
    virtual QImageIOHandler* create(QIODevice* device, const QByteArray& format = QByteArray()) const override;
};
#endif
