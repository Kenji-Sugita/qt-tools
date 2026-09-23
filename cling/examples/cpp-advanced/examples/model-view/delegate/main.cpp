#include <QApplication>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QStandardItemModel>
#include <QKeyEvent>

class CountryDelegate :public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CountryDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    QWidget* createEditor (QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        if (index.column() == 1) {
            QWidget* const editor = QStyledItemDelegate::createEditor(parent, option, index);
            editor->setAutoFillBackground(true);  // Workaround to avoid traslucent background
            return editor;
        }

        const auto editor = new QComboBox(parent);
        editor->installEventFilter(const_cast<CountryDelegate*>(this));
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        if (index.column() == 1) {
            QStyledItemDelegate::setEditorData(editor, index);
        } else {
            const auto comboBox = static_cast<QComboBox*>(editor);
            comboBox->addItems(countries());
            const int currentIndex = CountryDelegate::countries().indexOf(index.data(Qt::DisplayRole).toString());
            comboBox->setCurrentIndex(currentIndex);
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {
        if (index.column() == 1) {
            QStyledItemDelegate::setModelData(editor, model, index);
        } else {
            const auto comboBox = static_cast<QComboBox*>(editor);
            model->setData(index, comboBox->currentText());
        }
    }

     void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
     {
        if (index.column() == 1) {
            QStyledItemDelegate::updateEditorGeometry(editor, option, index);
        } else {
            // Just a silly example, don't allow the editor to get a smaller height than its sizehint.
            const int hCell = option.rect.height();
            const int hEditor = editor->sizeHint().height();
            const int h = qMax(hCell, hEditor);
            const QSize editorSize(option.rect.size().width(), h);
            editor->setGeometry(QRect(option.rect.topLeft() - QPoint(0, (h - hCell)/2), editorSize));
        }
    }

    bool eventFilter(QObject* object, QEvent* event) override
    {
        if (event->type() == QEvent::KeyRelease && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Return) {
            emit commitData(static_cast<QWidget*>(object));
            emit closeEditor(static_cast<QWidget*>(object), EditNextItem);
        }
        return false;
    }

    static QStringList countries()
    {
        static const QStringList countries {
            "Denmark", "Sweden", "Norway", "USA",
            "Germany", "Poland", "Iceland", "Holland",
            "Great Britain", "Ireland", "Scotland",
        };

        return countries;
    }
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    const std::vector<int> populations {
        5432, 9001, 4593, 295734,
        82431, 38635, 296, 16407,
        60441, 4015, 5062,
    };

    QStandardItemModel model(11, 2);
    for (int row = 0; row < 11; ++row) {
        model.setData(model.index(row, 0, QModelIndex()), CountryDelegate::countries().at(row));
        model.setData(model.index(row, 1, QModelIndex()), populations.at(row));
        model.setData(model.index(row, 1, QModelIndex()), QFlags<Qt::AlignmentFlag>::Int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
    }
    model.setHeaderData(0, Qt::Horizontal, "Country");
    model.setHeaderData(1, Qt::Horizontal, "Population");

    QListView list;
    list.setWindowTitle("QListView");
    list.setModel(&model);
    list.setItemDelegate(new CountryDelegate(&list));    // Should not share the same instance of a delegate.
    list.show();

    QTableView table;
    table.setWindowTitle("QTableView");
    table.setModel(&model);
    table.setItemDelegate(new CountryDelegate(&table));  // Should not share the same instance of a delegate.
    table.show();

    QTreeView tree;
    tree.setWindowTitle("QTreeView");
    tree.setModel(&model);
    tree.setItemDelegate(new CountryDelegate(&tree));    // Should not share the same instance of a delegate.
    tree.show();

    return app.exec();
}

#include "main.moc"
