#include "AnkerFormLayout.h"

#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QWidget>

using namespace Anker;

QFrame *makePane(const QString &text)
{
    auto *pane = new QFrame;
    pane->setFrameShape(QFrame::StyledPanel);
    pane->setMinimumSize(80, 80);
    auto *layout = new FormLayout(pane);
    auto *label = new QLabel(text);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label, {
        .left = attachForm(),
        .right = attachForm(),
        .top = attachForm(),
        .bottom = attachForm(),
    });
    return pane;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Anker Position Anchors");

    auto *layout = new FormLayout(&window);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setFractionBase(100);

    auto *leftPane = makePane("0-50%");
    auto *rightTopPane = makePane("50-100% / top");
    auto *rightBottomPane = makePane("50-100% / bottom");

    layout->addWidget(leftPane, {
        .left = attachForm(),
        .right = attachPosition(50, 6),
        .top = attachForm(),
        .bottom = attachForm(),
    });
    layout->addWidget(rightTopPane, {
        .left = attachPosition(50, 6),
        .right = attachForm(),
        .top = attachForm(),
        .bottom = attachPosition(50, 6),
    });
    layout->addWidget(rightBottomPane, {
        .left = attachPosition(50, 6),
        .right = attachForm(),
        .top = attachPosition(50, 6),
        .bottom = attachForm(),
    });

    window.resize(560, 360);
    window.show();
    return app.exec();
}
