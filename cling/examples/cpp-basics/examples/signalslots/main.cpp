#include "emitter.h"
#include "receiver.h"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Emitter emitter("Hello World");
    Receiver receiver;
    QObject::connect(&emitter, &Emitter::aSignal, &receiver, &Receiver::aSlot);

    emitter.show();

    return app.exec();
}
