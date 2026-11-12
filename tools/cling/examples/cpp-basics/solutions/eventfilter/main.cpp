#include <QApplication>
#include <QMainWindow>
#include <QStatusBar>
#include <QCheckBox>
#include <QLineEdit>
#include <QLayout>
#include <QMap>

class HelpFilter
    : public QObject
{
    Q_OBJECT

public:
    explicit HelpFilter(QStatusBar* statusBar, QObject* parent = nullptr)
        : QObject(parent) {
        this->statusBar = statusBar;
    }

    void registerHelp(QObject* object, const QString& helpMessage) {
        helpMessages.insert(object, helpMessage);
        connect(object, &QObject::destroyed, this, &HelpFilter::unregister);
    }

public slots:
    void unregister(QObject* object) {
        helpMessages.remove(object);
    }

protected:
    bool eventFilter(QObject* receiver, QEvent* event) {
        switch (event->type()) {
        case QEvent::Enter:
            if (helpMessages.contains(receiver)) {
                statusBar->showMessage(helpMessages.value(receiver), 5000);
            }
            break;
        case QEvent::Leave:
            if (!statusBar->currentMessage().isEmpty()) {
                statusBar->clearMessage();
            }
            break;
        default:
            break;
        }

        return false;
    }

private:
    QMap<QObject*, QString> helpMessages;
    QStatusBar* statusBar;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    const auto globalHelpFilter = new HelpFilter(mainWindow.statusBar(), &app);
    app.installEventFilter(globalHelpFilter);

    const auto checkBox = new QCheckBox("CheckBox");
    globalHelpFilter->registerHelp(checkBox, "Text for checkbox");

    const auto lineEdit= new QLineEdit;
    globalHelpFilter->registerHelp(lineEdit, "Text for lineEdit");

    const auto widget = new QWidget;
    mainWindow.setCentralWidget(widget);

    const auto topLayout = new QVBoxLayout(widget);
    topLayout->addWidget(checkBox);
    topLayout->addWidget(lineEdit);
    topLayout->addStretch();

    mainWindow.show();

    return app.exec();
}
#include "main.moc"
