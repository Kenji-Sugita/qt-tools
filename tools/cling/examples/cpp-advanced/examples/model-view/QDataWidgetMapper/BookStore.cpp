#include "BookStore.h"
#include "ui_BookStore.h"
#include <QDataWidgetMapper>
// QTBUG-69332
// [REG 5.10 -> 5.11][macOS] QLineEdit/QLabel is not updated properly when text in it is changed using QDataWidgetMapper
#if defined(Q_OS_MACOS) && QT_VERSION >= QT_VERSION_CHECK(5, 11, 0) && QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
#include <QTimer>
#endif

BookStore::BookStore(QAbstractItemModel* model, QWidget* parent)
    : QWidget(parent), ui(new Ui::BookStore)
{
    ui->setupUi(this);

    const auto mapper = new QDataWidgetMapper(this);
    mapper->setModel(model);
    mapper->addMapping(ui->title, 0);
    mapper->addMapping(ui->author, 1);
    mapper->addMapping(ui->price, 2);

    connect(ui->first, &QToolButton::clicked, mapper, &QDataWidgetMapper::toFirst);
    connect(ui->previous, &QToolButton::clicked, mapper, &QDataWidgetMapper::toPrevious);
    connect(ui->next, &QToolButton::clicked, mapper, &QDataWidgetMapper::toNext);
    connect(ui->last, &QToolButton::clicked, mapper, &QDataWidgetMapper::toLast);
    mapper->toFirst();
}
