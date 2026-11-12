#include "icpp/ReplSessionImpl.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QLibraryInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>
#include <QXmlStreamReader>

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <utility>

namespace icpp {

void ReplSessionImpl::showRegisteredSourceFiles()
{
    if (registeredSourceFilePaths.isEmpty()) {
        standardOutput << trMessage(QStringLiteral("登録ファイルはありません。"),
                                    QStringLiteral("No registered files."))
                       << Qt::endl;
        return;
    }

    const int lineNumberWidth = QString::number(registeredSourceFilePaths.size()).size();
    for (int index = 0; index < registeredSourceFilePaths.size(); ++index) {
        standardOutput << QStringLiteral("%1  %2")
                              .arg(index + 1, lineNumberWidth)
                              .arg(registeredSourceFilePaths.at(index))
                       << Qt::endl;
    }
}

bool ReplSessionImpl::editRegisteredSourceFileOrder()
{
    if (registeredSourceFilePaths.size() < 2) {
        standardOutput << trMessage(QStringLiteral("登録ファイルが 2 個未満なので順番変更は不要です。"),
                                    QStringLiteral("Fewer than two registered files; no reordering needed."))
                       << Qt::endl;
        return true;
    }

    QTemporaryFile orderFile(QDir::tempPath() + QStringLiteral("/icpp_files_XXXXXX.txt"));
    if (!orderFile.open()) {
        reportError(trMessage(QStringLiteral("順番編集用の一時ファイルを開けません。"),
                              QStringLiteral("Cannot open a temporary file for file order editing.")));
        return false;
    }

    QTextStream outputStream(&orderFile);
    outputStream << trMessage(
                        QStringLiteral("# 登録ファイルの順番を編集します。1 行に 1 ファイルパスを書いてください。\n"
                                       "# # で始まる行と空行は無視します。\n"
                                       "# 保存して閉じると、この順番で .r を実行します。\n\n"),
                        QStringLiteral("# Reorder registered files. Keep one file path per line.\n"
                                       "# Lines starting with # and empty lines are ignored.\n"
                                       "# Save and close to run .r with this order.\n\n"));
    for (const QString& filePath : registeredSourceFilePaths) {
        outputStream << filePath << Qt::endl;
    }
    outputStream.flush();

    const QString orderFilePath = orderFile.fileName();
    orderFile.close();

    if (!runEditorForFile(orderFilePath)) {
        return false;
    }

    const std::optional<QString> editedText = readTextFile(orderFilePath);
    if (!editedText.has_value()) {
        return false;
    }

    QStringList reorderedFilePaths;
    const QStringList lines = editedText->split(QLatin1Char('\n'));
    for (const QString& line : lines) {
        const QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty() || trimmedLine.startsWith(QLatin1Char('#'))) {
            continue;
        }

        const QString absoluteFilePath = QFileInfo(trimmedLine).absoluteFilePath();
        if (!registeredSourceFilePaths.contains(absoluteFilePath)) {
            reportError(trMessage(QStringLiteral("登録されていないファイルが含まれています: %1"),
                                  QStringLiteral("File is not registered: %1"))
                            .arg(trimmedLine));
            return false;
        }
        if (reorderedFilePaths.contains(absoluteFilePath)) {
            reportError(trMessage(QStringLiteral("同じファイルが複数回含まれています: %1"),
                                  QStringLiteral("File appears more than once: %1"))
                            .arg(trimmedLine));
            return false;
        }
        reorderedFilePaths.append(absoluteFilePath);
    }

    if (reorderedFilePaths.size() != registeredSourceFilePaths.size()) {
        reportError(trMessage(QStringLiteral("登録ファイルが不足しています。全ての登録ファイルを 1 回ずつ残してください。"),
                              QStringLiteral("Registered files are missing. Keep every registered file exactly once.")));
        return false;
    }

    registeredSourceFilePaths = reorderedFilePaths;
    standardOutput << trMessage(QStringLiteral("登録ファイルの順番を更新しました。"),
                                QStringLiteral("Registered file order updated."))
                   << Qt::endl;
    showRegisteredSourceFiles();
    return true;
}

void ReplSessionImpl::addRegisteredSourceFile(const QString& filePathArgument, const QString& historyEntry)
{
    const QString filePath = filePathArgument.trimmed();
    if (filePath.isEmpty()) {
        reportError(trMessage(QStringLiteral(".add にはファイルパスが必要です。"),
                              QStringLiteral(".add requires a file path.")));
        return;
    }

    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        reportError(trMessage(QStringLiteral("%1: ファイルが見つかりません。"),
                              QStringLiteral("%1: File not found."))
                        .arg(filePath));
        return;
    }
    if (!isAddableSourceFile(fileInfo)) {
        reportError(trMessage(
            QStringLiteral("%1: .add には .cpp などの実装ファイルを指定してください。header は実装ファイルから include します。"),
            QStringLiteral("%1: .add expects an implementation file such as .cpp. Include headers from that implementation file."))
                        .arg(filePath));
        return;
    }

    const QString absoluteFilePath = fileInfo.absoluteFilePath();
    if (registeredSourceFilePaths.contains(absoluteFilePath)) {
        standardOutput << trMessage(QStringLiteral("登録済みです: %1"),
                                    QStringLiteral("Already registered: %1"))
                              .arg(absoluteFilePath)
                       << Qt::endl;
        addHistoryEntry(historyEntry);
        return;
    }

    registeredSourceFilePaths.append(absoluteFilePath);
    rememberTouchedSourceFile(absoluteFilePath);
    standardOutput << trMessage(QStringLiteral("登録しました: %1"),
                                QStringLiteral("Added: %1"))
                          .arg(absoluteFilePath)
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::dropRegisteredSourceFile(const QString& filePathOrNumber, const QString& historyEntry)
{
    const QString target = filePathOrNumber.trimmed();
    if (target.isEmpty()) {
        reportError(trMessage(QStringLiteral(".drop にはファイルパスまたは番号が必要です。"),
                              QStringLiteral(".drop requires a file path or number.")));
        return;
    }

    bool isNumber = false;
    const int fileNumber = target.toInt(&isNumber);
    int index = -1;
    if (isNumber) {
        if (fileNumber <= 0 || fileNumber > registeredSourceFilePaths.size()) {
            reportError(trMessage(QStringLiteral(".drop の番号が範囲外です。"),
                                  QStringLiteral(".drop number is out of range.")));
            return;
        }
        index = fileNumber - 1;
    } else {
        const QString absoluteFilePath = QFileInfo(target).absoluteFilePath();
        index = registeredSourceFilePaths.indexOf(absoluteFilePath);
        if (index < 0) {
            reportError(trMessage(QStringLiteral("ファイルは登録されていません: %1"),
                                  QStringLiteral("File is not registered: %1"))
                            .arg(target));
            return;
        }
    }

    const QString removedFilePath = registeredSourceFilePaths.takeAt(index);
    standardOutput << trMessage(QStringLiteral("登録を外しました: %1"),
                                QStringLiteral("Dropped: %1"))
                          .arg(removedFilePath)
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

std::optional<QString> ReplSessionImpl::resolveRegisteredSourceFilePath(const QString& filePathOrNumber,
                                                       const QString& commandName)
{
    const QString target = filePathOrNumber.trimmed();
    if (target.isEmpty()) {
        reportError(trMessage(QStringLiteral("%1 にはファイルパスまたは番号が必要です。"),
                              QStringLiteral("%1 requires a file path or number."))
                        .arg(commandName));
        return std::nullopt;
    }

    bool isNumber = false;
    const int fileNumber = target.toInt(&isNumber);
    if (isNumber) {
        if (fileNumber <= 0 || fileNumber > registeredSourceFilePaths.size()) {
            reportError(trMessage(QStringLiteral("%1 の番号が範囲外です。"),
                                  QStringLiteral("%1 number is out of range."))
                            .arg(commandName));
            return std::nullopt;
        }
        return registeredSourceFilePaths.at(fileNumber - 1);
    }

    const QString absoluteFilePath = QFileInfo(target).absoluteFilePath();
    if (!registeredSourceFilePaths.contains(absoluteFilePath)) {
        reportError(trMessage(QStringLiteral("ファイルは登録されていません: %1"),
                              QStringLiteral("File is not registered: %1"))
                        .arg(target));
        return std::nullopt;
    }
    return absoluteFilePath;
}

void ReplSessionImpl::clearRegisteredSourceFiles()
{
    if (registeredSourceFilePaths.isEmpty()) {
        standardOutput << trMessage(QStringLiteral("登録ファイルはありません。"),
                                    QStringLiteral("No registered files."))
                       << Qt::endl;
        return;
    }

    if (registeredSourceFilePaths.size() > 1
        && !confirmYesNo(trMessage(
            QStringLiteral("警告: 複数の登録ファイルを全て外します。続けますか? [Y/N] "),
            QStringLiteral("Warning: multiple registered files will be cleared. Continue? [Y/N] ")))) {
        return;
    }

    registeredSourceFilePaths.clear();
    standardOutput << trMessage(QStringLiteral("登録ファイルを全て外しました。"),
                                QStringLiteral("Registered files cleared."))
                   << Qt::endl;
}

} // namespace icpp
