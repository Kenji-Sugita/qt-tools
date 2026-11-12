/********************************************************************************
** Form generated from reading UI file 'twobuttonswithspacer.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TWOBUTTONSWITHSPACER_H
#define UI_TWOBUTTONSWITHSPACER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TwoButtonsWithSpacer
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *messageLabel;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *okButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancellButton;
    QSpacerItem *horizontalSpacer_3;

    void setupUi(QWidget *TwoButtonsWithSpacer)
    {
        if (TwoButtonsWithSpacer->objectName().isEmpty())
            TwoButtonsWithSpacer->setObjectName("TwoButtonsWithSpacer");
        TwoButtonsWithSpacer->resize(400, 300);
        verticalLayout = new QVBoxLayout(TwoButtonsWithSpacer);
        verticalLayout->setObjectName("verticalLayout");
        messageLabel = new QLabel(TwoButtonsWithSpacer);
        messageLabel->setObjectName("messageLabel");

        verticalLayout->addWidget(messageLabel);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        okButton = new QPushButton(TwoButtonsWithSpacer);
        okButton->setObjectName("okButton");
        okButton->setEnabled(true);

        horizontalLayout->addWidget(okButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancellButton = new QPushButton(TwoButtonsWithSpacer);
        cancellButton->setObjectName("cancellButton");

        horizontalLayout->addWidget(cancellButton);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout);

        verticalLayout->setStretch(0, 1);

        retranslateUi(TwoButtonsWithSpacer);

        QMetaObject::connectSlotsByName(TwoButtonsWithSpacer);
    } // setupUi

    void retranslateUi(QWidget *TwoButtonsWithSpacer)
    {
        TwoButtonsWithSpacer->setWindowTitle(QCoreApplication::translate("TwoButtonsWithSpacer", "Two Button with Spacer", nullptr));
        messageLabel->setText(QString());
        okButton->setText(QCoreApplication::translate("TwoButtonsWithSpacer", "OK", nullptr));
        cancellButton->setText(QCoreApplication::translate("TwoButtonsWithSpacer", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TwoButtonsWithSpacer: public Ui_TwoButtonsWithSpacer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TWOBUTTONSWITHSPACER_H
