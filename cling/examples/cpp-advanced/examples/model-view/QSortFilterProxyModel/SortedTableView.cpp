#include "SortedTableView.h"
#include <QLabel>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QLayout>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

SortedTableView::SortedTableView(QWidget* parent)
    : QWidget(parent)
{
    const auto topLayout = new QGridLayout(this);

    const auto label = new QLabel(this);
    topLayout->addWidget(label, 0, 0);

    lineEdit = new QLineEdit(this);
    label->setText("Filter:");
    topLayout->addWidget(lineEdit, 0, 1);

    tableView = new QTableView(this);
    tableView->setSortingEnabled(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(tableView->horizontalHeader(), &QHeaderView::sectionClicked, this, &SortedTableView::setFilterColumn);
    topLayout->addWidget(tableView, 1, 0, 1, 2);

    filterProxyModel = new QSortFilterProxyModel(this);
    filterProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    filterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(lineEdit, &QLineEdit::textChanged, filterProxyModel, &QSortFilterProxyModel::setFilterWildcard);

    setFilterColumn(0);
}

SortedTableView::~SortedTableView()
{
    delete filterProxyModel;
}

void SortedTableView::setModel(QAbstractItemModel* model)
{
    filterProxyModel->setSourceModel(model);
    tableView->setModel(filterProxyModel);
    tableView->resizeColumnsToContents();
}

void SortedTableView::setFilterColumn(int column)
{
    filterProxyModel->setFilterKeyColumn(column);
    lineEdit->setFocus();
    lineEdit->clear();
}
