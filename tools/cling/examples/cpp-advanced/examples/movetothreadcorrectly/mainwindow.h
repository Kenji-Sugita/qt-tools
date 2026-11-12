#pragma once

#include <QWidget>
#include <QCloseEvent>
#include "worker.h"
#include "ui_mainwindow.h"

class QThread;

class MainWindow : public QWidget, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

public slots:
    void updateResult(int i);
    void cleanup(const QString& message);

protected:
   void closeEvent(QCloseEvent* event) override;

private slots:
    void startClicked();

private:
    Worker* m_worker;
    QThread* m_thread;
};
