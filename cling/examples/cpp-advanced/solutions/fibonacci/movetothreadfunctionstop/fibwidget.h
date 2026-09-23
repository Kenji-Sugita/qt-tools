#ifndef FIBWIDGET_H
#define FIBWIDGET_H

#include <QWidget>
class FibObject;
class QListWidget;

class FibWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FibWidget(QWidget* parent = nullptr);

public slots:
    void slotAddFib(int fib);
    void slotQuit();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QListWidget* listbox;
    QThread* workerThread;
    FibObject* fibObject;
    bool hasQuitBeenCalled = false;
};
#endif
