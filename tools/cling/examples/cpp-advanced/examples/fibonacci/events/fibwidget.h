#ifndef FIBWIDGET_H
#define FIBWIDGET_H

#include <QWidget>

class QListWidget;
class FibThread;

class FibWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FibWidget(QWidget* parent = nullptr);

protected:
    void customEvent(QEvent*) override;
    void closeEvent(QCloseEvent*) override;

private slots:
    void quit();

private:
    QListWidget* listbox;
    FibThread* workerThread;
    bool hasQuitBeenCalled = false;
};
#endif
