#include "findDialog.h"
#include <QProcess>
#include <QFileDialog>

FindDialog::FindDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    abortButton->setEnabled(false);

    process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &FindDialog::endOfProcess);
    connect(process, &QProcess::readyReadStandardOutput, this, &FindDialog::readStdout);
    connect(process, &QProcess::readyReadStandardError, this, &FindDialog::readStderr);
}

void FindDialog::on_editButton_clicked()
{
    const QString dir = QFileDialog::getExistingDirectory(this, QString(), rootDir->text());
    if (!dir.isNull()) {
        rootDir->setText(dir);
    }
}

void FindDialog::on_goButton_clicked()
{
    QStringList arguments;

    if (!rootDir->text().isEmpty()) {
        arguments << rootDir->text();
    } else {
        arguments << ".";
    }

    arguments << "-type" << "f" << "-name" << filePattern->text();

    process->start("find", arguments);

    abortButton->setEnabled(true);
    goButton->setEnabled(false);
    results->clear();
}

void FindDialog::endOfProcess()
{
    abortButton->setEnabled(false);
    goButton->setEnabled(true);
}

void FindDialog::readStdout()
{
    process->setReadChannel(QProcess::StandardOutput);
    while (process->canReadLine()) {
        QString line = process->readLine();
        line.chop(1);  // Remove a trailing newline
        results->append(line);
    }
}

void FindDialog::readStderr()
{
    process->setReadChannel(QProcess::StandardError);
    while (process->canReadLine()) {
        QString line = process->readLine();
        line.chop(1);  // Remove a trailing newline
        results->append(QString(R"(<font color="red">%1</font>)").arg(line));
    }
}

void FindDialog::terminateProcess()
{
    process->terminate();
}

void FindDialog::on_quitButton_clicked()
{
    terminateProcess();
    done(0);
}

void FindDialog::on_abortButton_clicked()
{
    terminateProcess();
}
