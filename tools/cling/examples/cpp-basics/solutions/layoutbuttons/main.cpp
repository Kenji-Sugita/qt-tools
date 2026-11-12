#include <QApplication>
#include <QPushButton>
#include <QLayout>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget top;
    const auto topLayout = new QVBoxLayout(&top);
    QPushButton* buttonA;
    QPushButton* buttonB;
    QPushButton* buttonC;
    QHBoxLayout* layout;

    // Without any modifications
    buttonA = new QPushButton("Button A");
    buttonB = new QPushButton("Button B");
    buttonC = new QPushButton("Button C");

    layout = new QHBoxLayout;
    layout->addWidget(buttonA);
    layout->addWidget(buttonB);
    layout->addWidget(buttonC);
    topLayout->addLayout(layout);

    // Exercise 1
    buttonA = new QPushButton("Button A");
    buttonB = new QPushButton("Button B");
    buttonC = new QPushButton("Button C");

    layout = new QHBoxLayout;
    layout->addWidget(buttonA, 2);
    layout->addWidget(buttonB, 1);
    layout->addWidget(buttonC, 1);
    topLayout->addLayout(layout);

    // Exercise 2
    buttonA = new QPushButton("Button A");
    buttonB = new QPushButton("Button B");
    buttonC = new QPushButton("Button C");

    layout = new QHBoxLayout;
    layout->addWidget(buttonA, 1);
    layout->addWidget(buttonB);
    layout->addWidget(buttonC);
    topLayout->addLayout(layout);

    // Exercise 3
    buttonA = new QPushButton("Button A");
    buttonB = new QPushButton("Button B");
    buttonC = new QPushButton("Button C");

    layout = new QHBoxLayout;
    layout->addWidget(buttonA);
    layout->addWidget(buttonB);
    layout->addWidget(buttonC);
    layout->addStretch();
    topLayout->addLayout(layout);

    // Exercise 4
    buttonA = new QPushButton("Button A");
    buttonB = new QPushButton("Button B");
    buttonC = new QPushButton("Button C");

    layout = new QHBoxLayout;
    layout->addWidget(buttonA);
    layout->addStretch();
    layout->addWidget(buttonB);
    layout->addStretch();
    layout->addWidget(buttonC);
    topLayout->addLayout(layout);

    // Change the vertical size policy of all QPushButtons.
    for (const auto button : top.findChildren<QPushButton*>()) {
        const QSizePolicy::Policy horizPolicy = button->sizePolicy().horizontalPolicy();
        button->setSizePolicy(QSizePolicy(horizPolicy, QSizePolicy::MinimumExpanding));
    }

    top.resize(600, 400);
    top.show();

    return app.exec();
}

