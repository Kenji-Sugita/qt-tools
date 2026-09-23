#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLayout>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget top;

    const auto nameLabel = new QLabel("Name:");
    const auto nameEdit = new QLineEdit;
    nameEdit->setText("Jay K. Hacker");

    const auto streetLabel = new QLabel("Street:");
    const auto streetEdit = new QLineEdit;
    streetEdit->setText("Santa Claus Street 24");

    const auto countryLabel = new QLabel("Country:");
    countryLabel->setAlignment(Qt::AlignCenter);

    const auto countries = new QListWidget;
    countries->addItems({ "Denmark",
                          "Greenland",
                          "Sweden",
                          "Norway",
                          "Finland",
                          "France",
                          "Germany",
                          "Italy",
                          "Rusia",
                          "Estonia" });

    const auto commentLabel = new QLabel("Comments:");
    commentLabel->setAlignment(Qt::AlignCenter);

    const auto edit = new QTextEdit;
    edit->setText("A programmer known world-wide");

    const auto ok = new QPushButton("&OK");
    const auto cancel = new QPushButton("&Cancel");


    const auto nameLayout = new QHBoxLayout;
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);

    const auto streetLayout = new QHBoxLayout;
    streetLayout->addWidget(streetLabel);
    streetLayout->addWidget(streetEdit);

    const auto buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(ok);
    buttonLayout->addWidget(cancel);

    const auto topLayout = new QVBoxLayout(&top);
    topLayout->addLayout(nameLayout);
    topLayout->addLayout(streetLayout);
    topLayout->addWidget(countryLabel);
    topLayout->addWidget(countries);
    topLayout->addWidget(commentLabel);
    topLayout->addWidget(edit);
    topLayout->addLayout(buttonLayout);

    top.show();

    return app.exec();
}
