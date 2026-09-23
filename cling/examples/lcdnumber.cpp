#include <QLCDNumber>
#include <QVBoxLayout>
#include <QDebug>

QLCDNumber* create_lcdnumber() {
    auto window = new QWidget{};
    auto topLayout = new QVBoxLayout(window);

    auto lcdnumber = new QLCDNumber{};
    topLayout->addWidget(lcdnumber);
    topLayout->setContentsMargins(20, 20, 20, 20);

    window->show();
    window->raise();
    window->activateWindow();

    return lcdnumber;
}

auto lcdnumber = create_lcdnumber();
