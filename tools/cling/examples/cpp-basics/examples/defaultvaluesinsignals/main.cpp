#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <QDebug>

enum class ButtonType {DefaultSignal, ValueSignal};
enum class RepaintOperation {NoRepaint, AlsoRepaint};

class TypedPushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit TypedPushButton(ButtonType type, QWidget* parent = nullptr)
        : QPushButton(parent) {
        switch (type) {
        case ButtonType::DefaultSignal:
            connect(this, &TypedPushButton::clicked, this, &TypedPushButton::emitDefault);
            setText("Default (AlsoRepaint)");
            break;
        case ButtonType::ValueSignal:
            connect(this, &TypedPushButton::clicked, this, &TypedPushButton::emitValue);
            setText("With Value (NoRepaint)");
            break;
        }
    }

signals:
    void doUpdate(RepaintOperation operation = RepaintOperation::AlsoRepaint);

public slots:
    void defaultSlot(int number = 0) {
        qDebug() << Q_FUNC_INFO;
        qDebug() << "number =" << number;
    }

private slots:
    void emitDefault() {
        emit doUpdate();
    }

    void emitValue() {
        emit doUpdate(RepaintOperation::NoRepaint);
    }
};

class Receiver : public QLabel
{
    Q_OBJECT

public:
    explicit Receiver(QWidget* parent = nullptr)
        : QLabel(parent) {
        setAlignment(Qt::AlignCenter);
        setText("Push a Button");
    }

public slots:
    void updateSlot(RepaintOperation operation) {
        switch (operation) {
        case RepaintOperation::AlsoRepaint:
            setText("AlsoRepaint");
            break;
        case RepaintOperation::NoRepaint:
            setText("NoRepaint");
            break;
        }
    }
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    const auto defaultSignalButton = new TypedPushButton(ButtonType::DefaultSignal);
    const auto valueSignalButton = new TypedPushButton(ButtonType::ValueSignal);
    const auto result = new Receiver;

    QObject::connect(defaultSignalButton, &TypedPushButton::doUpdate, result, &Receiver::updateSlot);
    QObject::connect(valueSignalButton, &TypedPushButton::doUpdate, result, &Receiver::updateSlot);

    QObject::connect(&app, SIGNAL(aboutToQuit()), defaultSignalButton, SLOT(defaultSlot()));
    // The new connection syntax introduced in Qt 5 cannot be used instead of the old-style syntax above.
    // QObject::connect(&app, &QApplication::aboutToQuit, defaultSignalButton, &TypedPushButton::defaultSlot);


    QWidget window;
    const auto topLayout = new QVBoxLayout(&window);
    topLayout->addWidget(defaultSignalButton);
    topLayout->setAlignment(defaultSignalButton, Qt::AlignHCenter);
    topLayout->addWidget(valueSignalButton);
    topLayout->setAlignment(valueSignalButton, Qt::AlignHCenter);
    topLayout->addWidget(result, 1, Qt::AlignHCenter);

    window.show();

    return app.exec();
}

#include "main.moc"
