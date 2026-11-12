#include <QApplication>
#include <QListWidget>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Simple ListMode without icons
    {
        const auto listWidget = new QListWidget;
        for (int i = 1; i < 10; ++i) {
            listWidget->addItem(QString("Item %1").arg(i));
        }
        listWidget->setAttribute(Qt::WA_DeleteOnClose);
        listWidget->show();
    }

    // ListMode with Icons
    {
        const auto listWidget = new QListWidget;
        for (int i = 1; i < 10; ++i) {
            const auto item = new QListWidgetItem(QString("Item %1").arg(i), listWidget);
            item->setIcon(QPixmap(QString(":/images/%1.png").arg(i)));
        }
        listWidget->setAttribute(Qt::WA_DeleteOnClose);
        listWidget->show();
    }

    // IconMode
    {
        const auto listWidget = new QListWidget;
        for (int i = 1; i < 10; ++i) {
            const auto item = new QListWidgetItem(QString("Item %1").arg(i), listWidget);
            item->setIcon(QPixmap(QString(":images/%1.png").arg(i)));
        }
        listWidget->setViewMode(QListView::IconMode);
        listWidget->setAttribute(Qt::WA_DeleteOnClose);
        listWidget->show();
    }

    return app.exec();
}
