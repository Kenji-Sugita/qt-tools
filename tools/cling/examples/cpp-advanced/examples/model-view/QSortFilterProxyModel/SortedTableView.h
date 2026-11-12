#ifndef SORTEDTABLEVIEW_H
#define SORTEDTABLEVIEW_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QTableView;
class QGridLayout;
class QAbstractItemModel;
class QSortFilterProxyModel;

class SortedTableView : public QWidget
{
    Q_OBJECT

public:
    explicit SortedTableView(QWidget* parent = nullptr);
    ~SortedTableView();

    void setModel(QAbstractItemModel* model);

private slots:
    void setFilterColumn(int column);

private:
    QSortFilterProxyModel* filterProxyModel;
    QTableView* tableView;
    QLineEdit* lineEdit;
};
#endif
