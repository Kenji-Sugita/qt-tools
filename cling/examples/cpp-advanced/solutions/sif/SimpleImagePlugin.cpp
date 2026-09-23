#include "SimpleImagePlugin.h"

#include "SimpleImageIOHandler.h"

QImageIOPlugin::Capabilities SimpleImagePlugin::capabilities(QIODevice* device, const QByteArray& format) const
{
    // First operate on the format
    if (format == "sif") {
        return Capabilities(CanRead | CanWrite);
    }

    if (!format.isEmpty()) {
        return Capabilities();
    }

    // ...but we might just have the device....
    if (!device->isOpen()) {
        return Capabilities();
    }

    Capabilities cap;
    if (device->isReadable() && SimpleImageIOHandler::canRead(device)) {
        cap |= CanRead;
    }
    if (device->isWritable()) {
        cap |= CanWrite;
    }
    return cap;

}

QImageIOHandler* SimpleImagePlugin::create(QIODevice* device, const QByteArray& format) const
{
    const auto handler = new SimpleImageIOHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}
