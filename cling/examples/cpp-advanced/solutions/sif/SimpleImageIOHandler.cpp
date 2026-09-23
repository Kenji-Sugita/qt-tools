#include "SimpleImageIOHandler.h"
#include <QImage>
#include <QTextStream>

bool SimpleImageIOHandler::canRead() const
{
    return canRead(device());
}

bool SimpleImageIOHandler::supportsOption(ImageOption option) const
{
    if (option == Size) {
	return true;
    }

    return false;
}

QVariant SimpleImageIOHandler::option(ImageOption option) const
{
    QSize size;

    if (option ==  Size) {
	if (device()) {
	    // Save  current position in the device
	    const qint64 oldPos = device()->pos();

	    //In the SimpleImage Format the 1st list is SIF
	    // The 2nd line is the size
	    const QString formatLine = QString(device()->readLine().data());
            Q_UNUSED(formatLine);

	    const QString sizeLine = QString(device()->readLine().data());
	    const QStringList sizeList = sizeLine.split(" ");
	    const int width = sizeList.at(0).toInt();
	    const int height = sizeList.at(1).toInt();

	    size.setWidth(width);
	    size.setHeight(height);

	    // If the device can be rolled back, do so...
	    if (!device()->isSequential()) {
		device()->seek(oldPos);
	    }
	}
    }

    return size;
}

bool SimpleImageIOHandler::read(QImage* image)
{
    if (!device()) return false;

    // Save  current position in the device
    const qint64 oldPos = device()->pos();

    //In the SimpleImage Format the 1st token is SIF
    QString formatLine;
    int width, height;

    QTextStream stream(device());
    stream >> formatLine >> height >> width;

    // qDebug("w:%d, h:%d", width, height);

    *image = QImage(width,height, QImage::Format_RGB32);

    //Ok now the format is made up of a token for each R-G-B component of each pixel
    for(int y = 0; y < height; y++) {
	for(int x = 0; x < width; x++) {
	    int r, g, b;
	    stream >> r >> g >> b;
	    // qDebug("(%d,%d) = r:%d, g:%d, b:%d", x, y, r, g, b);
	    image->setPixel(x, y, qRgb(r, g, b));
	}
    }

    // If the device can be rolled back, do so...
    if (!device()->isSequential()) {
	device()->seek(oldPos);
    }
    return true;
}

bool SimpleImageIOHandler::write(const QImage & image)
{
    QTextStream stream(device());
    int width = image.width();
    int height = image.height();


#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    stream << "SIF" << Qt::endl
	   << height << Qt::endl
	   << width << Qt::endl;
#else
    stream << "SIF" << endl
	   << height << endl
	   << width << endl;
#endif

    for (int y=0; y < height; y++) {
	for (int x = 0; x < width; x++) {
	    QRgb rgb = image.pixel(x, y);
	    stream << qRed(rgb) << " "
		   << qGreen(rgb) << " "
		   << qBlue(rgb) << " ";
	}
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	stream << Qt::endl;
#else
	stream << endl;
#endif
    }
    return false;
}

/* static */
bool SimpleImageIOHandler::canRead(QIODevice* device)
{
   // Bail if bad device pointer
   if (!device) {
      return false;
   }

   // If this is our format header... we are good.
   if (device->peek(3) == "SIF") {
      return true;
   }

   //Fall through for other format headers
   return false;
}
