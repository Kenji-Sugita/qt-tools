#include "fibobject.h"
#include "fibwidget.h"
#include <QListWidget>
#include <QPushButton>
#include <QLayout>
#include <QThread>
#include <QCloseEvent>
#include <QApplication>

FibWidget::FibWidget(QWidget* parent)
    : QWidget(parent)
{
    listbox = new QListWidget(this);

    const auto quit = new QPushButton(tr("Quit"), this);

    const auto layout = new QVBoxLayout(this);
    layout->addWidget(listbox);
    layout->addWidget(quit, 0, Qt::AlignCenter);

    fibObject = new FibObject;
    workerThread = new QThread(this);
    Q_ASSERT(QThread::currentThread() == fibObject->thread());
    fibObject->moveToThread(workerThread);
    connect(fibObject, &FibObject::fibFound, this, &FibWidget::slotAddFib);
    connect(fibObject, &FibObject::stopped, this, &FibWidget::slotQuit);
    connect(quit, &QPushButton::clicked, fibObject, &FibObject::stop);
    connect(this, &FibWidget::closed, fibObject, &FibObject::stop);
    connect(workerThread, &QThread::started, fibObject, &FibObject::start);
    workerThread->start(QThread::IdlePriority);
}

void FibWidget::slotAddFib(int fib)
{
    Q_ASSERT(QThread::currentThread() == thread());

    listbox->addItem(QString::number(fib));
    listbox->scrollToItem(listbox->item(listbox->count() - 1));
}

void FibWidget::slotQuit()
{
    workerThread->quit();
    workerThread->wait();
    delete fibObject;
    fibObject = 0;
    delete workerThread;
    workerThread = 0;

    // In Qt 6, calling QApplication::quit() now triggers a close event. Here's how to handle it.
    hasQuitBeenCalled = true;
    qApp->quit();
}

void FibWidget::closeEvent(QCloseEvent* event)
{
    if (hasQuitBeenCalled) {
        return;
    }
    emit closed();
    event->ignore();
}
