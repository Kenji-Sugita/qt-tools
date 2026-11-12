#ifndef BOOKSTORE_H
#define BOOKSTORE_H

#include <QWidget>
class QAbstractItemModel;

namespace Ui { class BookStore; }

class BookStore : public QWidget
{
    Q_OBJECT

public:
    explicit BookStore(QAbstractItemModel* model, QWidget* parent = nullptr);

private:
    Ui::BookStore* ui;
};
#endif
