#include "puzzle.h"
#include <QApplication>
#include <QPushButton>
#include <QLayout>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    const auto puzzle = new Puzzle;
    const auto next = new QPushButton("Next");
    next->setFocusPolicy(Qt::StrongFocus);
    QObject::connect(next, &QPushButton::clicked, puzzle, &Puzzle::showNext);

    const auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(next);

    const auto topLayout = new QVBoxLayout;
    topLayout->addWidget(puzzle);
    topLayout->addLayout(buttonLayout);
    topLayout->addStretch();

    QWidget top;
    top.setLayout(topLayout);
    top.show();

    return app.exec();
}
