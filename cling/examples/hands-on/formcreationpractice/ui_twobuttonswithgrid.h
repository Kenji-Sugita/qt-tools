/********************************************************************************
** Form generated from reading UI file 'twobuttonswithgrid.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TWOBUTTONSWITHGRID_H
#define UI_TWOBUTTONSWITHGRID_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TwoButtonsWithGrid
{
public:
    QGridLayout *gridLayout;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QLabel *messageLabel;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *TwoButtonsWithGrid)
    {
        if (TwoButtonsWithGrid->objectName().isEmpty())
            TwoButtonsWithGrid->setObjectName("TwoButtonsWithGrid");
        TwoButtonsWithGrid->resize(400, 300);
        gridLayout = new QGridLayout(TwoButtonsWithGrid);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(-1, -1, -1, 20);
        okButton = new QPushButton(TwoButtonsWithGrid);
        okButton->setObjectName("okButton");

        gridLayout->addWidget(okButton, 1, 0, 1, 1, Qt::AlignRight);

        cancelButton = new QPushButton(TwoButtonsWithGrid);
        cancelButton->setObjectName("cancelButton");

        gridLayout->addWidget(cancelButton, 1, 2, 1, 1, Qt::AlignLeft);

        messageLabel = new QLabel(TwoButtonsWithGrid);
        messageLabel->setObjectName("messageLabel");

        gridLayout->addWidget(messageLabel, 0, 0, 1, 3);

        horizontalSpacer = new QSpacerItem(40, 0, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer, 1, 1, 1, 1);

        gridLayout->setColumnStretch(0, 1);
        gridLayout->setColumnStretch(2, 1);

        retranslateUi(TwoButtonsWithGrid);

        QMetaObject::connectSlotsByName(TwoButtonsWithGrid);
    } // setupUi

    void retranslateUi(QWidget *TwoButtonsWithGrid)
    {
        TwoButtonsWithGrid->setWindowTitle(QCoreApplication::translate("TwoButtonsWithGrid", "Twe Buttons with Grid", nullptr));
        okButton->setText(QCoreApplication::translate("TwoButtonsWithGrid", "OK", nullptr));
        cancelButton->setText(QCoreApplication::translate("TwoButtonsWithGrid", "Cancel", nullptr));
        messageLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class TwoButtonsWithGrid: public Ui_TwoButtonsWithGrid {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TWOBUTTONSWITHGRID_H
