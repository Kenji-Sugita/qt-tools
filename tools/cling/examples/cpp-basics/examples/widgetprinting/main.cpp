#include <QApplication>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QLayout>
#include <QDebug>

class Printing : public QWidget
{
    Q_OBJECT

public:
    explicit Printing(QWidget* parent = nullptr)
        : QWidget(parent) {
        const auto checkBox = new QCheckBox("Hi");
        const auto edit = new QLineEdit;
        const auto printButton = new QPushButton("Print");
        connect(printButton, &QPushButton::clicked, this, &Printing::print);

        const auto frameLayout = new QHBoxLayout;
        frameLayout->addWidget(checkBox);
        frameLayout->addWidget(edit);

        const auto topLayout = new QVBoxLayout(this);
        topLayout->addLayout(frameLayout);
        topLayout->addWidget(printButton, 0, Qt::AlignHCenter);

        topLayout->addStretch();

        const auto quitButton = new QPushButton("Quit");
        topLayout->addWidget(quitButton, 0, Qt::AlignHCenter);
        connect(quitButton, &QPushButton::clicked, &QApplication::quit);
    }

private slots:
    void print() {
        const QPixmap grabbedPixmap = this->grab();
        const auto screenShot = new QLabel;
        screenShot->setAttribute(Qt::WA_DeleteOnClose);
        screenShot->setPixmap(grabbedPixmap);
        screenShot->show();
        connect(screenShot, &QLabel::destroyed, [](){ qDebug() << "destroyed"; });
    }
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Printing top;
    top.show();

    return app.exec();
}
#include "main.moc"
