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
    const auto buttonLayout = new QHBoxLayout{};
    buttonLayout->addStretch();
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch();

    const auto topLayout = new QVBoxLayout{};
    topLayout->addWidget(messageLabel);
    topLayout->addLayout(buttonLayout);

    setLayout(topLayout);

    connect(quitButton, &QPushButton::clicked, this, &MessageWidget::quitButtonClicked);
}

void MessageWidget::quitButtonClicked()
{
    emit quit();
}
