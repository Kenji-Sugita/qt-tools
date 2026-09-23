/********************************************************************************
** Form generated from reading UI file 'onebutton.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ONEBUTTON_H
#define UI_ONEBUTTON_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OneButton
{
public:
    QVBoxLayout *verticalLayout;
    QPushButton *okButton;

    void setupUi(QWidget *OneButton)
    {
        if (OneButton->objectName().isEmpty())
            OneButton->setObjectName("OneButton");
        OneButton->resize(400, 300);
        OneButton->setMinimumSize(QSize(0, 0));
        verticalLayout = new QVBoxLayout(OneButton);
        verticalLayout->setObjectName("verticalLayout");
        okButton = new QPushButton(OneButton);
        okButton->setObjectName("okButton");

        verticalLayout->addWidget(okButton, 0, Qt::AlignHCenter);


        retranslateUi(OneButton);

        QMetaObject::connectSlotsByName(OneButton);
    } // setupUi

    void retranslateUi(QWidget *OneButton)
    {
        OneButton->setWindowTitle(QCoreApplication::translate("OneButton", "One Button", nullptr));
        okButton->setText(QCoreApplication::translate("OneButton", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class OneButton: public Ui_OneButton {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ONEBUTTON_H
