#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget top;

    const auto nameLabel = new QLabel("Username:");
    const auto nameEdit = new QLineEdit;

    const auto passwordLabel = new QLabel("Password:");
    const auto passwordEdit = new QLineEdit;

    const auto topLayout = new QVBoxLayout(&top);
    const auto nameLayout = new QHBoxLayout;
    const auto passwordLayout = new QHBoxLayout;

    topLayout->addLayout(nameLayout);
    topLayout->addLayout(passwordLayout);

    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);

    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(passwordEdit);

    top.show();

    return app.exec();
}
