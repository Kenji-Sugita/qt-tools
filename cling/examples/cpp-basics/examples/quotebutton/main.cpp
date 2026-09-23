#include "quotebutton.h"
#include <QApplication>
#include <QTextEdit>
#include <QLayout>
//#include <QErrorMessage>

int main(int argc, char** argv)
{
    QApplication app(argc,argv);
//    QErrorMessage::qtHandler();

    QWidget top;
    top.setWindowTitle("Quotes from Office Space");

    const auto button = new QuoteButton("Quote");
    const auto edit = new QTextEdit;
    edit->setReadOnly(true);
    QObject::connect(button, &QuoteButton::quote, edit, &QTextEdit::setText);

    const auto topLayout = new QVBoxLayout(&top);
    topLayout->addWidget(button, 0, Qt::AlignHCenter);
    topLayout->addWidget(edit);

    button->setFocus();

    top.show();

    return app.exec();
}
