#ifndef SIMPLEIMAGEIOHANDLER_H
#define SIMPLEIMAGEIOHANDLER_H

#include <QImageIOHandler>
#include <QSize>
#include <QVariant>

class SimpleImageIOHandler : public QImageIOHandler
{
public:
    virtual bool canRead() const override;
    virtual bool supportsOption(ImageOption option) const override;
    virtual QVariant option(ImageOption option) const override;
    virtual bool read(QImage* image) override;
    virtual bool write(const QImage& image) override;

    // Convenience
    static bool canRead(QIODevice* device);
};
#endif
