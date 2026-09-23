#include <QApplication>
#include <QWidget>
#include <QDebug>

#include "ui_onebutton.h"
#include "ui_onebuttonwithspacer.h"
#include "ui_twobuttons.h"
#include "ui_twobuttonswithspacer.h"
#include "ui_twobuttonswithgrid.h"

int main(int argc, char **argv)
{
    QApplication app{argc, argv};

    auto oneButton = new QWidget{};
    auto oneButtonUi = new Ui_OneButton{};
    oneButtonUi->setupUi(oneButton);
    oneButton->show();

    auto oneButtonWithSpacer = new QWidget{};
    auto oneButtonWithSpacerUi = new Ui_OneButtonWithSpacer{};
    oneButtonWithSpacerUi->setupUi(oneButtonWithSpacer);
    oneButtonWithSpacer->show();

    auto twoButtons = new QWidget{};
    auto twoButtonsUi = new Ui_TwoButtons{};
    twoButtonsUi->setupUi(twoButtons);
    twoButtons->show();

    auto twoButtonsWithSpacer = new QWidget{};
    auto twoButtonsWithSpacerUi = new Ui_TwoButtonsWithSpacer{};
    twoButtonsWithSpacerUi->setupUi(twoButtonsWithSpacer);
    twoButtonsWithSpacer->show();

    auto twoButtonsWithGrid = new QWidget{};
    auto twoButtonsWithGridUi = new Ui_TwoButtonsWithGrid{};
    twoButtonsWithGridUi->setupUi(twoButtonsWithGrid);
    twoButtonsWithGrid->show();

    return app.exec();
}
