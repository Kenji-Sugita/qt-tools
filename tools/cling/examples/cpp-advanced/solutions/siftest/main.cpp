#include <QApplication>
#include <QLabel>

const char* sifData =
"SIF\n"
"3 3\n"
"000 000 000 255 000 000 000 255 000\n"
"255 255 255 255 255 255 255 255 255\n"
"128 128 128 128 128 128 128 128 128\n";

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QLabel label;
#if defined(Q_OS_MACOS)
    label.setPixmap(QPixmap("../../../../sif/sif/64x64.sif"));
    //label.setPixmap(QPixmap(app.applicationDirPath() + "/../../../../../sif/sif/64x64.sif"));
#elif defined(Q_OS_WINDOWS)
    label.setPixmap(QPixmap("../../sif/sif/64x64.sif"));
#else
    label.setPixmap(QPixmap("../sif/sif/64x64.sif"));
#endif
    label.show();

    const QImage sifImage = QImage::fromData(reinterpret_cast<const uchar*>(sifData), qstrlen(sifData));
    const QPixmap sifPixmap = QPixmap::fromImage(sifImage);
    QLabel sifLabel;
    sifLabel.setPixmap(sifPixmap);
    sifLabel.resize(64, 64);
    sifLabel.show();

    return app.exec();
}
