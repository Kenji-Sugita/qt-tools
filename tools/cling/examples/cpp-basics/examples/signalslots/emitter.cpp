#include "emitter.h"

Emitter::Emitter(const QString& label, QWidget* parent)
    : QPushButton(label, parent)
{
    connect(this, &Emitter::clicked, this, &Emitter::aSignal);
}
