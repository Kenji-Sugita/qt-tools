#include <QApplication>
#include <QListView>
#include <vector>
#include <QRangeModel>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    std::vector<int> data = {1, 2, 3, 4, 5};

    QRangeModel model(&data);

    QObject::connect(&model, &QRangeModel::dataChanged,
                     [&]() {
                        for (int value : data) {
                            qDebug() << value;
                        }
                     });

    QListView view;
    view.setModel(&model);
    view.show();

    return app.exec();
}
