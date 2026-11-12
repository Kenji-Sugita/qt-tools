#include <QApplication>
#include <QTreeWidget>
#include <QTreeWidgetItem>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QTreeWidget tree;
    tree.setSortingEnabled(true);
#if 1
    tree.setHeaderLabels({ "Column A", "Column B", "Column C" });
#else
    tree.setHeaderItem(new QTreeWidgetItem({ "Column A", "Column B", "Column C" }));
#endif

#if 1
    const auto topItem = new QTreeWidgetItem(&tree);
    topItem->setText(0, "Top A");
    topItem->setText(1, "Top B");
    topItem->setText(2, "Top C");

    QTreeWidgetItem* const topItem2 = new QTreeWidgetItem(&tree);
    topItem2->setText(0, "Second Top A");
    topItem2->setText(1, "Second Top B");
    topItem2->setText(2, "Second Top C");
#else
    tree.addTopLevelItem(new QTreeWidgetItem({ "Top A", "Top B", "Top C" }));

    tree.addTopLevelItem(new QTreeWidgetItem({ "Second Top A", "Second Top B", "Second Top C" }));
    const auto topItem2 = tree.topLevelItem(1);
#endif

#if 1
    for (int i = 1; i < 10; ++i) {
        QTreeWidgetItem* const subItem = new QTreeWidgetItem(topItem2);
        subItem->setText(0, QString("Sub A - %1").arg(i));
        subItem->setText(1, QString("Sub B - %1").arg(i));
        subItem->setText(2, QString("Sub C - %1").arg(i));
        subItem->setIcon(1, QPixmap(QString(":/images/%1.png").arg(i)));
    }
#else
    for (int i = 1; i < 10; ++i) {
        topItem2->addChild(new QTreeWidgetItem({ QString("Sub A - %1").arg(i), QString("Sub B - %1").arg(i), QString("Sub C - %1").arg(i) }));
        QTreeWidgetItem* const subItem = topItem2->child(0);
        if (subItem) {
            subItem->setIcon(1, QPixmap(QString(":/images/%1.png").arg(i)));
        }
    }
#endif

    tree.expandToDepth(1);
    for (int column = 0; column < tree.columnCount(); ++column) {
        tree.resizeColumnToContents(column);
    }

    tree.show();

    return app.exec();
}
