#include <QApplication>
#include <QListView>
#include <QRangeModel>   // Qt 6.10 以降

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // 1, 2, 3, 4, 5 を二乗して 1, 4, 9, 16, 25 を生成する view
    auto square = [](int i) { return i * i; };
    // このモデルは変更できない。
    auto range = std::views::iota(1, 6) | std::views::transform(square);

    QRangeModel model(range);

    QListView view;
    view.setModel(&model);
    view.setWindowTitle("QRangeModel");
    view.resize(320, 240);
    view.show();

    return app.exec();
}
