#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTableView>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QLabel>
#include <QDebug>

class Selection : public QMainWindow
{
    Q_OBJECT

public:
    explicit Selection(QWidget* parent = nullptr)
        : QMainWindow(parent) {
        currentItem = new QLabel;
        statusBar()->addWidget(currentItem);

        const auto view = new QTableView;
        setCentralWidget(view);

        model = new QStandardItemModel(10, 5, this);
        view->setModel(model);

        QItemSelectionModel* const selectionModel = view->selectionModel();

        connect(selectionModel, &QItemSelectionModel::currentChanged, this, &Selection::currentItemChanged);
        connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &Selection::updateSelection);

        // Select (0, 0) x (2, 2)
        QItemSelection selectRectangle(model->index(0, 0, QModelIndex()), model->index(2, 2, QModelIndex()));
        selectionModel->select(selectRectangle, QItemSelectionModel::Select);

        // Select 6-8 rows
        QItemSelection selectRows(model->index(6, 0, QModelIndex()), model->index(8, 0, QModelIndex()));
        selectionModel->select(selectRows, QItemSelectionModel::Select | QItemSelectionModel::Rows);

        // Toggle 1-1 column
        QItemSelection toggleColumn(model->index(0, 1, QModelIndex()), model->index(0, 1, QModelIndex()));
        selectionModel->select(toggleColumn, QItemSelectionModel::Columns  | QItemSelectionModel::Toggle);
    }

protected slots:
    void currentItemChanged(const QModelIndex& current) {
        currentItem->setText(QString("row: %1 col: %2").arg(current.row()).arg(current.column()));
    }

    void updateSelection(const QItemSelection& selected, const QItemSelection& deselected) {
        for (const QModelIndex& index : selected.indexes()) {
            model->setData(index, QString("(%1,%2)").arg(index.row()).arg(index.column()));
        }

        for (const QModelIndex& index : deselected.indexes()) {
            model->setData(index, QString());
        }
    }

private:
    QLabel* currentItem;
    QStandardItemModel* model;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Selection selection;
    selection.resize(800, 600);
    selection.show();

    return app.exec();
}

#include "main.moc"
