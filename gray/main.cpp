#include <QCoreApplication>
#include <QImage>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDebug>

QImage convertToGrayscale(const QImage& image)
{
    QImage grayImage = image.convertToFormat(QImage::Format_ARGB32);

    QRgb* rgb = reinterpret_cast<QRgb*>(grayImage.bits());
    const QRgb* rgbLimit = rgb + (grayImage.width() * grayImage.height());
    while (rgb < rgbLimit) {
        int gray = qGray(*rgb);
        *rgb = qRgba(gray, gray, gray, qAlpha(*rgb));
        ++rgb;
    }

    return grayImage;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    if (argc == 1) {
        QTextStream(stderr) << "Usage: gray file..." << Qt::endl;
        return -1;
    }

    for (int argn = 1; argn < argc; ++argn) {
        QString sourceFile = argv[ argn ];

        QImage sourceImage;
        if (!sourceImage.load(sourceFile)) {
            QTextStream(stderr) << "Cannot load: " << sourceFile << Qt::endl;
            continue;
        }

        QImage grayImage = convertToGrayscale(sourceImage);

        QFileInfo sourceFileInfo(sourceFile);
        QString dirName = sourceFileInfo.dir().path();
        QString baseName = sourceFileInfo.baseName();
        QString outFileName;
        int count = 0;
        while (count < 1000) {
            if (count == 0) {
                outFileName = dirName + "/" + baseName + "-gray.png";
            } else {
                outFileName = dirName + "/" + baseName + "-gray" + QString::number(count) + ".png";
	    }
            QFileInfo outFileInfo(outFileName);
            if (!outFileInfo.exists()) {
                break;
	    }
            count++;
        }

        if (!grayImage.save(outFileName)) {
            QTextStream(stderr) << "Caannot save: " << sourceFile << Qt::endl;
        }
    }
}
