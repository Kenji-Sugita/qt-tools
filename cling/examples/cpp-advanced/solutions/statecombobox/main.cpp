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
        if ((index > 4 && index < 8) || index == 9) {
            QStandardItem* const item = model->item(index, modelColumn());
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);

            QFont italicFont = font();
// QTBUG-69489 QComboBox doesn't show italic text as italic
#if defined(Q_OS_MACOS) && QT_VERSION >= QT_VERSION_CHECK(5, 11, 0) && QT_VERSION <= QT_VERSION_CHECK(5, 13, 0)
            italicFont = QFont("Helvetica");  // Workaround
#endif
            italicFont.setItalic(true);
            item->setData(italicFont, Qt::FontRole);  // Equivalent to setItemData(index, italicFont, Qt::FontRole)
        }
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
