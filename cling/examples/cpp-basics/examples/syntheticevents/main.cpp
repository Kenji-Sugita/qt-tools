#include <QApplication>
#include <QPushButton>
#include <QLayout>

class MasterButton : public QPushButton
{
    Q_OBJECT

public:
    explicit MasterButton(const QString& text, QPushButton* anotherButton, QWidget* parent = nullptr)
        : QPushButton(text, parent), anotherButton(anotherButton) {
    }

protected:
    bool event(QEvent* event) {
        if (QEvent::MouseButtonPress <= event->type() && event->type() <= QEvent::KeyRelease) {
            qApp->sendEvent(anotherButton, event);
        }
        return QPushButton::event(event);
    }

private:
    QPushButton* const anotherButton;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    const auto slaveBotton = new QPushButton("Slave");
    const auto masterButton = new MasterButton("Master", slaveBotton);

    QWidget top;
    const auto topLayout = new QVBoxLayout(&top);
    topLayout->addWidget(masterButton, 0, Qt::AlignHCenter);
    topLayout->addWidget(slaveBotton, 0, Qt::AlignHCenter);
    top.show();

    masterButton->setFocus();

    return app.exec();
}
#include "main.moc"
