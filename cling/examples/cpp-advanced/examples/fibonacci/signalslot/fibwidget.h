#ifndef FIBWIDGET_H
#define FIBWIDGET_H

#include <QWidget>
class FibThread;
class QListWidget;

class FibWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FibWidget(QWidget* parent = nullptr);

public slots:
    void addFib(int fib);
    void quit();

protected:
    void closeEvent(QCloseEvent*) override;

private:
    QListWidget* listbox;
    FibThread* workerThread;
    bool hasQuitBeenCalled = false;
};
#endif
