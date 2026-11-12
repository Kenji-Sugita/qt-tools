#include <QThread>
#include <QPushButton>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    stopButton->setEnabled(false);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startClicked);

    m_thread = new QThread(this);

    m_worker = new Worker;
    m_worker->moveToThread(m_thread);

    connect(stopButton, &QPushButton::clicked, m_worker, &Worker::stop);
    connect(m_worker, &Worker::dump, this, &MainWindow::updateResult);
    connect(m_worker, &Worker::finished, this, &MainWindow::cleanup);
    connect(m_worker, &Worker::finished, m_thread, &QThread::quit);
    connect(m_thread, &QThread::started, m_worker, &Worker::generate);
}

void MainWindow::startClicked()
{
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    if (!m_thread->isRunning()) {
        result->clear();
        m_thread->start();
    }
}

void MainWindow::updateResult(int i)
{
    result->setText(QString::number(i));
}

void MainWindow::cleanup(const QString& message)
{
    startButton->setEnabled(true);
    stopButton->setEnabled(false);
    if (m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait();
    }
    result->setText(message);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_thread && m_thread->isRunning()) {
        event->ignore();
    } else {
        event->accept();
    }
}
