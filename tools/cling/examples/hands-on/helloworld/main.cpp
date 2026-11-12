#include <QApplication>
#include <QLabel>

int main(int argc, char** argv)
{
    QApplication app{argc, argv};

    const QString message{"Hello World!"};
//    const QString message(R"{<div align="center">
//                                <span style="font-size:128pt;color:SteelBlue">
//                                    Hello World!
//                                </span>
//                             </div>)"};

    QLabel hello{message};
    hello.setMinimumSize(hello.sizeHint());
    hello.show();

    return app.exec();
}
