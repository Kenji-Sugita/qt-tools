#include <QApplication>
#include <QLabel>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QLabel label("Hello world", nullptr);
//    QLabel label(R"(<div align="center"><font color=red><b>Hello world</b></font></div>)", nullptr);
//    QLabel label(R"(<div align="center"><span style="font-size: 48pt; color: Red">Hello world</span></div>)", nullptr);

    label.setMinimumSize(label.sizeHint());
    label.show();

    return app.exec();
}
