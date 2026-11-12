#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>

QPushButton* create_button() {
    auto window = new QWidget{};
    auto topLayout = new QVBoxLayout{window};

    auto button = new QPushButton("OK?");
    QObject::connect(button, &QPushButton::clicked, []() { qDebug() << "OK?"; });
    topLayout->addWidget(button);
    topLayout->setAlignment(button, Qt::AlignHCenter);

    window->show();
    window->raise();
    window->activateWindow();

    return button;
}

auto button = create_button();
