#include <QApplication>
#include <QSlider>
#include <QHelpEvent>
#include <QToolTip>
#include <QLCDNumber>
#include <QLayout>

class SliderWithTip : public QSlider
{
    Q_OBJECT

public:
    explicit SliderWithTip(Qt::Orientation orient, QWidget* parent = nullptr)
        : QSlider(orient, parent) {
    }

protected:
    bool event(QEvent* event) {
        if (event->type() == QEvent::ToolTip) {
            const auto* const helpEvent = static_cast<QHelpEvent*>(event);
            QToolTip::showText(helpEvent->globalPos(), QString::number(value()));

            return true;
        }
        return QSlider::event(event);
    }
};

int main(int argc, char**argv)
{
    QApplication app(argc, argv);

    const auto slider = new SliderWithTip(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(50);

    const auto lcdNumber = new QLCDNumber;
    lcdNumber->display(slider->value());
    lcdNumber->setDigitCount(2);

    // The fouth argument is same as static_cast<void (QLCDNumber::*)(int)>(&QLCDNumber::display)).
    QObject::connect(slider, &SliderWithTip::valueChanged, lcdNumber, QOverload<int>::of(&QLCDNumber::display));

    QWidget top;
    const auto topLayout = new QVBoxLayout(&top);
    topLayout->addWidget(slider);
    topLayout->addWidget(lcdNumber);

    top.show();

    return app.exec();
}

#include "main.moc"
