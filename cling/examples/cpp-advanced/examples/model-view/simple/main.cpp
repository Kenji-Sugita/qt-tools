#include <QApplication>
#include <QSplitter>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QFileSystemModel>
#include <QTimer>

class FileSystemModel : public QFileSystemModel
{
public:
    explicit FileSystemModel(QObject* parent = nullptr)
        : QFileSystemModel(parent)
    {
    }

    bool hasChildren(const QModelIndex& parent) const override
    {
        QDir dir(filePath(parent));
        dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        return (dir.count() != 0);
    }
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QSplitter splitter;
    const auto list = new QListView(&splitter);
    const auto table = new QTableView(&splitter);
    const auto tree = new QTreeView(&splitter);

    FileSystemModel model;
    model.setRootPath(QDir::root().path());

    list->setModel(&model);
    table->setModel(&model);
    tree->setModel(&model);
#if defined(Q_OS_UNIX)
    list->setRootIndex(model.index("/etc"));
    table->setRootIndex(model.index("/etc"));
    tree->setRootIndex(model.index("/etc"));
#elif defined(Q_OS_WIN)
    list->setRootIndex(model.index("C:/Users"));
    table->setRootIndex(model.index("C:/Users"));
    tree->setRootIndex(model.index("C:/Users"));
#else
    table->setRootIndex(model.index("."));
#endif

    splitter.setSizes({ 250, 450, 300});
    splitter.show();

    QObject::connect(&model, &QFileSystemModel::directoryLoaded, table, &QTableView::resizeColumnsToContents);
    QObject::connect(&model, &QFileSystemModel::directoryLoaded, [&]() {
        tree->expandToDepth(1);
        for (int column = 0; column < tree->model()->columnCount(); ++column) {
            tree->resizeColumnToContents(column);
        }
    });

    return app.exec();
}
