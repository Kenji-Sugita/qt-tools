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
    layout->addWidget(buttonA);
    layout->addWidget(buttonB);
    layout->addWidget(buttonC);
    topLayout->addLayout(layout);

    // Exercise 2
    buttonA = new QPushButton("Button A");
    buttonB = new QPushButton("Button B");
    buttonC = new QPushButton("Button C");

    layout = new QHBoxLayout;
    layout->addWidget(buttonA);
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
    topLayout->addLayout(layout);

    // Exercise 4
    buttonA = new QPushButton("Button A");
    buttonB = new QPushButton("Button B");
    buttonC = new QPushButton("Button C");

    layout = new QHBoxLayout;
    layout->addWidget(buttonA);
    layout->addWidget(buttonB);
    layout->addWidget(buttonC);
    topLayout->addLayout(layout);

    top.resize(600, 400);
    top.show();

    return app.exec();
}

