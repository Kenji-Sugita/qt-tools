#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLayout>
#include <QDebug>

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget* parent = nullptr)
        : QDialog(parent) {
        const auto description = new QLabel("<p>This example shows that you should override accept and reject rather than "
                                            "implementing new slots like slotOK() and slotCancel().</p>"
                                            "<p>The reason is that pressing escape or pressing the window manager close button will make reject() fire, "
                                            "but of course slotCancel() would not fire, how would Qt know to hook the escape key up with that slot?</p>");
        description->setWordWrap(true);

        const auto edit = new QLineEdit;

        const auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttons, &QDialogButtonBox::accepted, this, &Dialog::slotOK);
        connect(buttons, &QDialogButtonBox::rejected, this, &Dialog::slotCancel);

        const auto topLayhout = new QVBoxLayout(this);
        topLayhout->addWidget(description);
        topLayhout->addWidget(edit);
        topLayhout->addWidget(buttons);
    }

public slots:
    void accept() {
        qDebug() << "accept";
        QDialog::accept();
    }

    void reject() {
        qDebug() << "reject";
        QDialog::reject();
    }

protected slots:
    void slotOK() {
        qDebug() << "slotOK";
        accept();
    }

    void slotCancel() {
        qDebug() << "slotCancel";
        reject();
    }
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Q_UNUSED(app);

    Dialog dialog;
    dialog.exec();
}

#include "main.moc"
