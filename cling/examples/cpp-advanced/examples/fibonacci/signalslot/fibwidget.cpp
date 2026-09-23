#include "fibwidget.h"
#include "fibthread.h"
#include <QListWidget>
#include <QPushButton>
#include <QLayout>
#include <QApplication>

FibWidget::FibWidget(QWidget* parent)
    : QWidget(parent)
{
    listbox = new QListWidget(this);
    listbox->setObjectName("listbox");

    QPushButton* const quit = new QPushButton(tr("Quit"), this);
    quit->setObjectName("quit");
    connect(quit, &QPushButton::clicked, this, &FibWidget::quit);

    QVBoxLayout* const topLayout = new QVBoxLayout(this);
    topLayout->addWidget(listbox);
    topLayout->addWidget(quit, 0, Qt::AlignCenter);

    workerThread = new FibThread(this);
    connect(workerThread, &FibThread::fibFound, this, &FibWidget::addFib);
    workerThread->start(QThread::IdlePriority);
}

void FibWidget::addFib(int fib)
{
    // Make sure that this slot is called in the GUI thread.
    Q_ASSERT(QThread::currentThread() == thread());

    listbox->addItem(QString::number(fib));
    listbox->scrollToItem(listbox->item(listbox->count() - 1));
}

void FibWidget::closeEvent(QCloseEvent*)
{
    if (hasQuitBeenCalled) {
        return;
    }
    quit();
}

void FibWidget::quit()
{
    if (workerThread->isRunning()) {
        workerThread->requestInterruption();
        workerThread->wait();
        delete workerThread;
        workerThread = nullptr;
    }

    // In Qt 6, calling QApplication::quit() now triggers a close event. Here's how to handle it.
    hasQuitBeenCalled = true;
    qApp->quit();
}
