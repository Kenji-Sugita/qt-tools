#include "AnkerFormLayout.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

using namespace Anker;

QLabel *makePanel(const QString &text, const QString &color)
{
    auto *label = new QLabel(text);
    label->setAlignment(Qt::AlignCenter);
    label->setMinimumSize(120, 56);
    label->setStyleSheet(QString("QLabel { background: %1; border: 1px solid #555; }").arg(color));
    return label;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const QSize initialWindowSize(640, 420);
    const QRect selfInitialGeometry(430, 292, 180, 72);
    constexpr int oppositeFormLeftMargin = 80;
    constexpr int oppositeFormRightFromLeftEdge = 370;

    QWidget window;
    window.setWindowTitle("Anker Full Attachments");
    window.resize(initialWindowSize);

    auto *layout = new FormLayout(&window);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(10);
    layout->setFractionBase(100);
    layout->setRubberPositioning(true);

    auto *formPanel = makePanel("FORM\nleft/top + right=50%", "#d7ecff");
    auto *spacingPanel = makePanel("WIDGET\nuses default spacing", "#dcf7df");
    auto *oppositePanel = makePanel("OPPOSITE_WIDGET\naligned to first row", "#fff0c9");
    auto *selfPanel = makePanel("SELF\ninitial rectangle", "#eadbff");
    auto *oppositeFormPanel = makePanel("OPPOSITE_FORM\nright from opposite left edge", "#ffd9d6");
    auto *button = new QPushButton("Bottom right");

    selfPanel->setMinimumSize(selfInitialGeometry.size());
    selfPanel->setGeometry(selfInitialGeometry);
    oppositeFormPanel->setMinimumSize(320, 72);

    layout->addWidget(formPanel, {
        .left = attachForm(),
        .right = attachPosition(50, 6),
        .top = attachForm(),
        .bottom = attachPosition(40, 5),
    });
    layout->addWidget(spacingPanel, {
        .left = attachWidget(formPanel),
        .right = attachForm(),
        .top = attachOppositeWidget(formPanel),
        .bottom = attachPosition(40, 5),
    });
    layout->addWidget(oppositePanel, {
        .left = attachOppositeWidget(formPanel),
        .right = attachOppositeWidget(spacingPanel),
        .top = attachWidget(formPanel),
        .bottom = attachPosition(65),
    });
    layout->addWidget(selfPanel, {
        .left = attachSelf(),
        .right = attachSelf(),
        .top = attachSelf(),
        .bottom = attachSelf(),
    });
    layout->addWidget(oppositeFormPanel, {
        .left = attachForm(oppositeFormLeftMargin),
        .right = attachOppositeForm(-oppositeFormRightFromLeftEdge),
        .top = attachPosition(72),
    });
    layout->addWidget(button, {
        .right = attachForm(),
        .bottom = attachForm(),
    });

    window.show();
    return app.exec();
}
