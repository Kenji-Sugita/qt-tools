#include <QLabel>
#include <QPushButton>
#include <QLayout>

#include "messagewidget.h"

MessageWidget::MessageWidget(const QString& message, QWidget* parent)
    : QWidget{parent}
{
    const auto messageLabel = new QLabel{message};
    messageLabel->setAlignment(Qt::AlignCenter);

    const auto quitButton = new QPushButton{"Quit"};

    const auto topLayout = new QVBoxLayout{this};
    topLayout->addWidget(messageLabel, 1);
    topLayout->addWidget(quitButton);
    topLayout->setAlignment(quitButton, Qt::AlignCenter);

    connect(quitButton, &QPushButton::clicked, this, &MessageWidget::quit);
}
