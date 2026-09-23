// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QPushButton を作成し、クリックされたら qDebug メッセージを出してください。
// go() 関数を用意し、main() は書かないでください。

#include <QDebug>
#include <QPushButton>

QPushButton* go()
{
    auto* button = new QPushButton("Click me");
    button->resize(220, 90);
    button->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(button, &QPushButton::clicked, []() {
        qDebug() << "button clicked";
    });
    button->show();
    button->raise();
    return button;
}
