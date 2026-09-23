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

void ReplSessionImpl::printExpression(const QString& expression, const QString& historyEntry)
{
    if (expression.trimmed().isEmpty()) {
        reportError(trMessage(QStringLiteral(".print には式が必要です。"),
                              QStringLiteral(".print requires an expression.")));
        return;
    }

    QString sourceCode;
    if (currentEngine == Engine::QtCling) {
        sourceCode = QStringLiteral("#include <QDebug>\nqDebug().noquote() << (%1);")
                         .arg(expression);
    } else {
        sourceCode = QStringLiteral("#include <iostream>\nstd::cout << (%1) << std::endl;")
                         .arg(expression);
    }

    interpreter.sendSource(sourceCode);
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::printExpressionType(const QString& expression, const QString& historyEntry)
{
    if (expression.trimmed().isEmpty()) {
        reportError(trMessage(QStringLiteral(".ptype には式が必要です。"),
                              QStringLiteral(".ptype requires an expression.")));
        return;
    }

    QString sourceCode;
    if (currentEngine == Engine::QtCling) {
        sourceCode = QStringLiteral(R"icpp(#include <QDebug>
#include <QMetaType>
#include <type_traits>
#include <typeinfo>
([]() {
using IcppType = std::remove_cv_t<std::remove_reference_t<decltype((%1))>>;
const char* typeName = QMetaType::fromType<IcppType>().name();
const auto prettyTypeName = [](auto*) {
    const QString pretty = QString::fromLatin1(__PRETTY_FUNCTION__);
    const QString marker = QStringLiteral(" = ");
    qsizetype start = pretty.lastIndexOf(marker);
    if (start < 0) {
        return pretty;
    }
    start += marker.size();
    const qsizetype end = pretty.indexOf(QLatin1Char(']'), start);
    return end < 0 ? pretty.mid(start) : pretty.mid(start, end - start);
};
const QString cppTypeName = prettyTypeName(static_cast<IcppType*>(nullptr));
const QString metaTypeName = typeName ? QString::fromLatin1(typeName) : QString();
QString displayTypeName = cppTypeName;
if (displayTypeName == QStringLiteral("QLatin1String")) {
    displayTypeName = QStringLiteral("QLatin1StringView");
} else if (displayTypeName == QStringLiteral("QBasicUtf8StringView<false>")
           || displayTypeName == QStringLiteral("QBasicUtf8StringView<true>")) {
    displayTypeName = QStringLiteral("QUtf8StringView");
}
QString displayMetaTypeName = metaTypeName;
if (displayMetaTypeName == QStringLiteral("QBasicUtf8StringView<false>")
    || displayMetaTypeName == QStringLiteral("QBasicUtf8StringView<true>")) {
    displayMetaTypeName = QStringLiteral("QUtf8StringView");
}
if (!displayMetaTypeName.isEmpty() && displayMetaTypeName != displayTypeName) {
    qDebug().noquote() << displayTypeName + QStringLiteral(" (meta: ") + displayMetaTypeName + QLatin1Char(')');
} else {
    qDebug().noquote() << displayTypeName;
}
})();)icpp")
                         .arg(expression);
    } else {
        sourceCode = QStringLiteral(R"(#include <iostream>
#include <type_traits>
#include <typeinfo>
([]() {
using IcppType = std::remove_cv_t<std::remove_reference_t<decltype((%1))>>;
std::cout << typeid(IcppType).name() << std::endl;
})();)")
                         .arg(expression);
    }

    interpreter.sendSource(sourceCode);
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::showDefinitions()
{
    standardOutput << trMessage(QStringLiteral("登録ファイル:"), QStringLiteral("registered files:"))
                   << Qt::endl;
    if (registeredSourceFilePaths.isEmpty()) {
        standardOutput << QStringLiteral("  %1")
                              .arg(trMessage(QStringLiteral("(なし)"), QStringLiteral("(none)")))
                       << Qt::endl;
    } else {
        const int lineNumberWidth = QString::number(registeredSourceFilePaths.size()).size();
        for (int index = 0; index < registeredSourceFilePaths.size(); ++index) {
            const QString& filePath = registeredSourceFilePaths.at(index);
            standardOutput << QStringLiteral("%1  %2")
                                  .arg(index + 1, lineNumberWidth)
                                  .arg(filePath)
                           << Qt::endl;
            const std::optional<QString> sourceCode = readTextFile(filePath);
            if (!sourceCode.has_value()) {
                continue;
            }
            const QDir baseDirectory = QFileInfo(filePath).absoluteDir();
            showDefinitionsForSource(*sourceCode, memberAccessMapForSource(*sourceCode, baseDirectory));
        }
    }

    standardOutput << trMessage(QStringLiteral("編集バッファ:"), QStringLiteral("edit buffer:"))
                   << Qt::endl;
    const QString sourceCode = visibleSourceCode();
    showDefinitionsForSource(sourceCode, memberAccessMapForSource(sourceCode, QDir::current()));
}

void ReplSessionImpl::showDefinitionsForSource(const QString& sourceCode, const MemberAccessMap& accessMap)
{
    const QList<SourceDefinition> definitions = findSourceDefinitions(sourceCode);
    if (definitions.isEmpty()) {
        standardOutput << QStringLiteral("   %1")
                              .arg(trMessage(QStringLiteral("(定義なし)"),
                                             QStringLiteral("(no definitions)")))
                       << Qt::endl;
        return;
    }

    const int lineNumberWidth = QString::number(definitions.last().lineNumber).size();
    for (const SourceDefinition& definition : definitions) {
        standardOutput << QStringLiteral("   %1  %2")
                              .arg(definition.lineNumber, lineNumberWidth)
                              .arg(definitionDescriptionWithAccess(definition, accessMap))
                       << Qt::endl;
    }
}

void ReplSessionImpl::evaluateWithoutAppending(const QString& sourceCode, const QString& historyEntry)
{
    if (sourceCode.trimmed().isEmpty()) {
        reportError(trMessage(QStringLiteral(".eval には C++ コードが必要です。"),
                              QStringLiteral(".eval requires C++ code.")));
        return;
    }

    interpreter.sendSource(sourceCode);
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::evaluateAndAppend(const QString& sourceCode, const QString& historyEntry)
{
    if (sourceCode.trimmed().isEmpty()) {
        reportError(trMessage(QStringLiteral(".append には C++ コードが必要です。"),
                              QStringLiteral(".append requires C++ code.")));
        return;
    }

    appendSource(sourceCode);
    interpreter.sendSource(sourceCode);
    addHistoryEntry(historyEntry);
}

} // namespace icpp
