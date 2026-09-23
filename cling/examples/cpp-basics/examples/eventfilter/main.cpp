#include <QApplication>
#include <QMainWindow>
#include <QStatusBar>
#include <QCheckBox>
#include <QLineEdit>
#include <QLayout>

class HelpFilter
    : public QObject
{
    Q_OBJECT

public:
    explicit HelpFilter(QStatusBar* statusBar, const QString& helpMessage, QObject* parent = nullptr)
        : QObject(parent) {
        this->statusBar = statusBar;
        this->helpMessage = helpMessage;
    }

private:
    QStatusBar* statusBar;
    QString helpMessage;

protected:
    bool eventFilter(QObject* /* receiver */, QEvent* event) {
        switch (event->type()) {
        case QEvent::Enter:
            statusBar->showMessage(helpMessage, 5000);
            break;
        case QEvent::Leave:
            statusBar->clearMessage();
            break;
        default:
            break;
        }

        return false;
    }
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QMainWindow mainWindow;

    const auto checkBox = new QCheckBox("CheckBox");
    auto const checkBoxFilter = new HelpFilter(mainWindow.statusBar(), "Text for checkbox", checkBox);
    checkBox->installEventFilter(checkBoxFilter);

    const auto lineEdit= new QLineEdit;
    const auto editFilter = new HelpFilter(mainWindow.statusBar(), "Text for lineEdit", lineEdit);
    lineEdit->installEventFilter(editFilter);

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
