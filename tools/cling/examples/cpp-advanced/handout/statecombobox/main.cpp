#include <QApplication>
#include <QStandardItemModel>
#include <QComboBox>

class StateComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit StateComboBox(QWidget* parent = nullptr);
};

StateComboBox::StateComboBox(QWidget* parent)
    : QComboBox(parent)
{
    const auto model = qobject_cast<QStandardItemModel*>(this->model());
    Q_ASSERT(model);

    for (int index = 0; index < 10; ++index) {
        addItem(QString("Index %1").arg(index));
    }
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    StateComboBox stateComboBox;
    stateComboBox.show();

    return app.exec();
}

#include "main.moc"
