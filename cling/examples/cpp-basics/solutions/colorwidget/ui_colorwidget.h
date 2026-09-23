/********************************************************************************
** Form generated from reading UI file 'colorwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORWIDGET_H
#define UI_COLORWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ColorWidget
{
public:
    QVBoxLayout *vboxLayout;
    QFrame *colorFrame;
    QGridLayout *gridLayout;
    QLabel *redLabel;
    QLabel *blueLabel;
    QLabel *greenLabel;
    QSlider *blueSlider;
    QSlider *greenSlider;
    QSlider *redSlider;

    void setupUi(QWidget *ColorWidget)
    {
        if (ColorWidget->objectName().isEmpty())
            ColorWidget->setObjectName("ColorWidget");
        ColorWidget->resize(264, 246);
        vboxLayout = new QVBoxLayout(ColorWidget);
        vboxLayout->setObjectName("vboxLayout");
        colorFrame = new QFrame(ColorWidget);
        colorFrame->setObjectName("colorFrame");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(colorFrame->sizePolicy().hasHeightForWidth());
        colorFrame->setSizePolicy(sizePolicy);
        colorFrame->setMinimumSize(QSize(240, 120));
        colorFrame->setFrameShape(QFrame::StyledPanel);
        colorFrame->setFrameShadow(QFrame::Raised);

        vboxLayout->addWidget(colorFrame);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        redLabel = new QLabel(ColorWidget);
        redLabel->setObjectName("redLabel");

        gridLayout->addWidget(redLabel, 0, 0, 1, 1);

        blueLabel = new QLabel(ColorWidget);
        blueLabel->setObjectName("blueLabel");

        gridLayout->addWidget(blueLabel, 2, 0, 1, 1);

        greenLabel = new QLabel(ColorWidget);
        greenLabel->setObjectName("greenLabel");

        gridLayout->addWidget(greenLabel, 1, 0, 1, 1);

        blueSlider = new QSlider(ColorWidget);
        blueSlider->setObjectName("blueSlider");
        blueSlider->setMaximum(255);
        blueSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(blueSlider, 2, 1, 1, 1);

        greenSlider = new QSlider(ColorWidget);
        greenSlider->setObjectName("greenSlider");
        greenSlider->setMaximum(255);
        greenSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(greenSlider, 1, 1, 1, 1);

        redSlider = new QSlider(ColorWidget);
        redSlider->setObjectName("redSlider");
        redSlider->setMaximum(255);
        redSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(redSlider, 0, 1, 1, 1);


        vboxLayout->addLayout(gridLayout);


        retranslateUi(ColorWidget);

        QMetaObject::connectSlotsByName(ColorWidget);
    } // setupUi

    void retranslateUi(QWidget *ColorWidget)
    {
        ColorWidget->setWindowTitle(QCoreApplication::translate("ColorWidget", "Color Widget", nullptr));
        redLabel->setText(QCoreApplication::translate("ColorWidget", "Red", nullptr));
        blueLabel->setText(QCoreApplication::translate("ColorWidget", "Blue", nullptr));
        greenLabel->setText(QCoreApplication::translate("ColorWidget", "Green", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ColorWidget: public Ui_ColorWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORWIDGET_H
