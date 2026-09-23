/********************************************************************************
** Form generated from reading UI file 'onebuttonwithspacer.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ONEBUTTONWITHSPACER_H
#define UI_ONEBUTTONWITHSPACER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OneButtonWithSpacer
{
public:
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *okButton;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *verticalSpacer_2;

    void setupUi(QWidget *OneButtonWithSpacer)
    {
        if (OneButtonWithSpacer->objectName().isEmpty())
            OneButtonWithSpacer->setObjectName("OneButtonWithSpacer");
        OneButtonWithSpacer->resize(400, 300);
        verticalLayout = new QVBoxLayout(OneButtonWithSpacer);
        verticalLayout->setObjectName("verticalLayout");
        verticalSpacer = new QSpacerItem(20, 109, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        okButton = new QPushButton(OneButtonWithSpacer);
        okButton->setObjectName("okButton");

        horizontalLayout->addWidget(okButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer_2 = new QSpacerItem(20, 109, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);


        retranslateUi(OneButtonWithSpacer);

        QMetaObject::connectSlotsByName(OneButtonWithSpacer);
    } // setupUi

    void retranslateUi(QWidget *OneButtonWithSpacer)
    {
        OneButtonWithSpacer->setWindowTitle(QCoreApplication::translate("OneButtonWithSpacer", "One Button with Spacer", nullptr));
        okButton->setText(QCoreApplication::translate("OneButtonWithSpacer", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class OneButtonWithSpacer: public Ui_OneButtonWithSpacer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ONEBUTTONWITHSPACER_H
