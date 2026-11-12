#include "fibwidget.h"
#include "fibthread.h"
#include "fibevent.h"
#include <QListWidget>
#include <QPushButton>
#include <QLayout>
#include <QApplication>

FibWidget::FibWidget(QWidget* parent)
    : QWidget(parent)
{
    listbox = new QListWidget;
    listbox->setObjectName("listbox");

    const auto quit = new QPushButton(tr("Quit"));
    quit->setObjectName("quit");
    connect(quit, &QPushButton::clicked, this, &FibWidget::quit);

    const auto topLayout = new QVBoxLayout(this);
    topLayout->addWidget(listbox);
    topLayout->addWidget(quit, 0, Qt::AlignCenter);

    workerThread = new FibThread(this, this);
    workerThread->start(QThread::IdlePriority);
}

void FibWidget::customEvent(QEvent* event)
{
    if (event->type() != FibEventType) {
        QWidget::customEvent(event);
        return;
    }

    const auto* const fibevent = static_cast<FibEvent*>(event);
    listbox->addItem(QString::number(fibevent->result()));
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
