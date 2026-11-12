#ifndef SIGNAL_H
#define SIGNAL_H
#include <QPushButton>

class Emitter : public QPushButton
{
    Q_OBJECT

public:
    explicit Emitter(const QString& label, QWidget* parent = nullptr);

signals:
    void aSignal();
    void anOtherSignal(int number, QWidget* widget);
};
#endif
