/********************************************************************************
** Form generated from reading UI file 'twobuttons.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TWOBUTTONS_H
#define UI_TWOBUTTONS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TwoButtons
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *messageLabel;
    QHBoxLayout *horizontalLayout;
    QPushButton *okButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelButton;

    void setupUi(QWidget *TwoButtons)
    {
        if (TwoButtons->objectName().isEmpty())
            TwoButtons->setObjectName("TwoButtons");
        TwoButtons->resize(400, 300);
        verticalLayout = new QVBoxLayout(TwoButtons);
        verticalLayout->setObjectName("verticalLayout");
        messageLabel = new QLabel(TwoButtons);
        messageLabel->setObjectName("messageLabel");

        verticalLayout->addWidget(messageLabel);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        okButton = new QPushButton(TwoButtons);
        okButton->setObjectName("okButton");

        horizontalLayout->addWidget(okButton, 0, Qt::AlignRight);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancelButton = new QPushButton(TwoButtons);
        cancelButton->setObjectName("cancelButton");

        horizontalLayout->addWidget(cancelButton, 0, Qt::AlignLeft);


        verticalLayout->addLayout(horizontalLayout);

        verticalLayout->setStretch(0, 1);

        retranslateUi(TwoButtons);

        QMetaObject::connectSlotsByName(TwoButtons);
    } // setupUi

    void retranslateUi(QWidget *TwoButtons)
    {
        TwoButtons->setWindowTitle(QCoreApplication::translate("TwoButtons", "Two Buttons", nullptr));
        messageLabel->setText(QString());
        okButton->setText(QCoreApplication::translate("TwoButtons", "OK", nullptr));
        cancelButton->setText(QCoreApplication::translate("TwoButtons", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TwoButtons: public Ui_TwoButtons {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TWOBUTTONS_H
