#include <QApplication>
#include <QCheckBox>
#include <QLayout>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget top;
    const auto checkBox1 = new QCheckBox("CheckBox1", &top);
    const auto checkBox2 = new QCheckBox("CheckBox2", &top);
#if 0
    auto sizePolicy2 = checkBox2->sizePolicy();
    sizePolicy2.setRetainSizeWhenHidden(true);
    checkBox2->setSizePolicy(sizePolicy2);
#endif
    const auto checkBox3 = new QCheckBox("CheckBox3", &top);

    const auto topLayout = new QHBoxLayout(&top);
    topLayout->addWidget(checkBox1);
    topLayout->addWidget(checkBox2);
    topLayout->addWidget(checkBox3);

    checkBox2->hide();
    top.show();
    top.hide();
    top.show();

    return app.exec();
}
