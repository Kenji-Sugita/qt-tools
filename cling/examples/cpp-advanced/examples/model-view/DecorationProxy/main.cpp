#include <QApplication>
#include <QListView>
#include <QIdentityProxyModel>
#include <QStringListModel>
#include <QStringList>

class ProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    explicit ProxyModel(QObject* parent = nullptr)
        : QIdentityProxyModel(parent)
    {
        pixmap = QPixmap(":/images/tux.png");
        pixmap = pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (index.isValid() && role == Qt::DecorationRole) {
            return pixmap;
        }
        return QIdentityProxyModel::data(index, role);
    }

private:
    QPixmap pixmap;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QStringListModel model({ "Denmark", "Norway", "Sweden", "USA", "Poland" });

    QListView listViewWithoutIcon;
    listViewWithoutIcon.setModel(&model);
    listViewWithoutIcon.setWindowTitle("QListView");
    listViewWithoutIcon.show();

    QListView listViewWithIcon;
    ProxyModel proxy;
    proxy.setSourceModel(&model);
    listViewWithIcon.setModel(&proxy);
    listViewWithIcon.setWindowTitle("QListView with icons");
    listViewWithIcon.show();

    return app.exec();
}

#include "main.moc"
