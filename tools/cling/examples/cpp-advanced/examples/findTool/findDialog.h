#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include "ui_findDialog.h"
#include <QDialog>
#include <QString>

class QProcess;

class FindDialog :public QDialog, private Ui::FindDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget* parent = nullptr);

protected slots:
    void on_editButton_clicked();
    void on_goButton_clicked();
    void on_quitButton_clicked();
    void on_abortButton_clicked();

    void endOfProcess();
    void readStdout();
    void readStderr();
    void terminateProcess();

private:
    QProcess* process;
};
#endif
