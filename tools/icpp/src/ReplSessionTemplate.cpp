#include "icpp/ReplSessionImpl.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>

#include <optional>

namespace icpp {

namespace {

struct TemplateFile {
    QString path;
    QString sourceCode;
};

QString xmlEscaped(QString text)
{
    text.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    text.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    text.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    text.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    text.replace(QLatin1Char('\''), QStringLiteral("&apos;"));
    return text;
}

QString sanitizedFileStem(QString text, const QString& newFileNameMode)
{
    text = text.trimmed();
    text.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_]+")), QStringLiteral("_"));
    text.replace(QRegularExpression(QStringLiteral("_+")), QStringLiteral("_"));
    text.remove(QRegularExpression(QStringLiteral("^_+|_+$")));
    if (text.isEmpty()) {
        return QStringLiteral("widget");
    }
    if (newFileNameMode == QStringLiteral("asis")) {
        return text;
    }
    return text.toLower();
}

QString classNameFromFileStem(const QString& fileStem)
{
    QString result;
    bool capitalizeNext = true;
    for (const QChar character : fileStem) {
        if (!character.isLetterOrNumber()) {
            capitalizeNext = true;
            continue;
        }
        if (result.isEmpty() && character.isDigit()) {
            result.append(QLatin1Char('_'));
        }
        const QString text(character);
        result.append(capitalizeNext ? text.toUpper() : text);
        capitalizeNext = false;
    }
    return result.isEmpty() ? QStringLiteral("Widget") : result;
}

QFileInfo templateBaseInfo(const QString& base, const QString& newFileNameMode)
{
    QFileInfo rawInfo(base);
    const QString parentPath = rawInfo.path() == QStringLiteral(".") ? QString() : rawInfo.path();
    const QString stem = sanitizedFileStem(rawInfo.completeBaseName(), newFileNameMode);
    const QString filePath = parentPath.isEmpty() ? stem : QDir(parentPath).filePath(stem);
    return QFileInfo(filePath);
}

QString classNameFromTemplateBase(const QString& base)
{
    const QFileInfo rawInfo(base);
    QString stem = rawInfo.completeBaseName().trimmed();
    if (stem.isEmpty()) {
        stem = QStringLiteral("widget");
    }
    return classNameFromFileStem(stem);
}

QString widgetHeaderSource(const QString& className)
{
    return QStringLiteral(R"(#pragma once

#include <QWidget>

class %1 : public QWidget {
    %2

public:
    explicit %1(QWidget *parent = nullptr);
};
)")
        .arg(className, QStringLiteral("Q_" "OBJECT"));
}

QString widgetCppSource(const QString& stem, const QString& className)
{
    return QStringLiteral(R"(#include "%1.h"

#include <QLabel>
#include <QVBoxLayout>

%2::%2(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    auto *label = new QLabel("Hello icpp", this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    setWindowTitle("%2");
    resize(260, 120);
}

%2 *go() {
    return new %2;
}

#include "%3"
)")
        .arg(stem, className, QStringLiteral("moc_%1.cpp").arg(stem));
}

QString uiFileSource(const QString& className)
{
    return QStringLiteral(R"(<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>%1</class>
 <widget class="QWidget" name="%1">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>360</width>
    <height>180</height>
   </rect>
  </property>
  <property name="windowTitle">
   <string>%1</string>
  </property>
  <layout class="QVBoxLayout" name="verticalLayout">
   <item>
    <widget class="QLabel" name="titleLabel">
     <property name="text">
      <string>Hello icpp</string>
     </property>
     <property name="alignment">
      <set>Qt::AlignCenter</set>
     </property>
    </widget>
   </item>
  </layout>
 </widget>
 <resources/>
 <connections/>
</ui>
)")
        .arg(xmlEscaped(className));
}

QString qrcFileSource()
{
    return QStringLiteral(R"(<RCC>
  <qresource prefix="/"/>
</RCC>
)");
}

QList<TemplateFile> filesForTemplate(const QString& kind,
                                     const QFileInfo& baseInfo,
                                     const QString& className)
{
    const QString directoryPath = baseInfo.path() == QStringLiteral(".") ? QString() : baseInfo.path();
    const QString stem = baseInfo.fileName();
    auto pathFor = [&](const QString& fileName) {
        return directoryPath.isEmpty() ? fileName : QDir(directoryPath).filePath(fileName);
    };

    if (kind == QStringLiteral("widget")) {
        return {{pathFor(stem + QStringLiteral(".h")), widgetHeaderSource(className)},
                {pathFor(stem + QStringLiteral(".cpp")), widgetCppSource(stem, className)}};
    }
    if (kind == QStringLiteral("ui")) {
        return {{pathFor(stem + QStringLiteral(".ui")), uiFileSource(className)}};
    }
    if (kind == QStringLiteral("qrc")) {
        return {{pathFor(stem + QStringLiteral(".qrc")), qrcFileSource()}};
    }
    return {};
}

void showTemplateNextSteps(QTextStream& output,
                           const QString& kind,
                           const QFileInfo& baseInfo)
{
    if (kind == QStringLiteral("widget")) {
        output << QStringLiteral("  .add %1").arg(baseInfo.filePath() + QStringLiteral(".cpp"))
               << Qt::endl;
        output << QStringLiteral("  .gen") << Qt::endl;
        output << QStringLiteral("  static auto w = go();") << Qt::endl;
        output << QStringLiteral("  w->show();") << Qt::endl;
        output << QStringLiteral("  w->raise();") << Qt::endl;
        return;
    }
    if (kind == QStringLiteral("ui")) {
        output << QStringLiteral("  .designer %1").arg(baseInfo.filePath() + QStringLiteral(".ui"))
               << Qt::endl;
        output << QStringLiteral("  .uiinfo %1").arg(baseInfo.filePath() + QStringLiteral(".ui"))
               << Qt::endl;
        output << QStringLiteral("  .gen") << Qt::endl;
        return;
    }
    output << QStringLiteral("  .qrc %1").arg(baseInfo.filePath() + QStringLiteral(".qrc"))
           << Qt::endl;
    output << QStringLiteral("  .gen") << Qt::endl;
}

void showTemplateUsageHint(QTextStream& output,
                           const QString& kind,
                           const QFileInfo& baseInfo,
                           const QString& formClassName,
                           const QString& title)
{
    if (kind == QStringLiteral("ui")) {
        const QString headerName = QStringLiteral("ui_%1.h").arg(baseInfo.fileName());
        output << QStringLiteral("") << Qt::endl;
        output << title << Qt::endl;
        output << QStringLiteral("  #include \"%1\"").arg(headerName) << Qt::endl;
        output << QStringLiteral("  class Widget : public QWidget, private Ui::%1 { ... };")
                      .arg(formClassName)
               << Qt::endl;
        output << QStringLiteral("  setupUi(this);") << Qt::endl;
        return;
    }
    if (kind == QStringLiteral("qrc")) {
        output << QStringLiteral("") << Qt::endl;
        output << title << Qt::endl;
        output << QStringLiteral("  #include \"qrc_%1.cpp\"").arg(baseInfo.fileName())
               << Qt::endl;
        output << QStringLiteral("  QFile file(\":/path/in/resource\");") << Qt::endl;
    }
}

QString defaultBaseForTemplateKind(const QString& kind)
{
    if (kind == QStringLiteral("ui")) {
        return QStringLiteral("Form");
    }
    if (kind == QStringLiteral("qrc")) {
        return QStringLiteral("resources");
    }
    return QStringLiteral("Widget");
}

QString promptForTemplateKind(const QString& kind)
{
    if (kind == QStringLiteral("ui")) {
        return QStringLiteral("フォーム名を入力してください [%1]: ");
    }
    if (kind == QStringLiteral("qrc")) {
        return QStringLiteral("resource 名を入力してください [%1]: ");
    }
    return QStringLiteral("widget class 名を入力してください [%1]: ");
}

QString englishPromptForTemplateKind(const QString& kind)
{
    if (kind == QStringLiteral("ui")) {
        return QStringLiteral("Form name [%1]: ");
    }
    if (kind == QStringLiteral("qrc")) {
        return QStringLiteral("Resource name [%1]: ");
    }
    return QStringLiteral("Widget class name [%1]: ");
}

} // namespace

void ReplSessionImpl::showTemplateHelp(const QString& historyEntry)
{
    standardOutput << trMessage(
        QStringLiteral(R"(.template <kind> [base]

kind:
  widget    Q_%1OBJECT と moc include を含む QWidget クラスを作成します
  ui        .ui ファイルだけを作成します
  qrc       .qrc ファイルだけを作成します

base:
  widget は class 名、ui は form 名、qrc は resource 名の元になる名前です。
  省略すると対話端末では入力を促します。
  空入力、または非対話実行では kind ごとの既定名を使います。

例:
  .template widget counter
  .template ui settings
  .template qrc resources

既存ファイルは上書きしません。作成前にファイル一覧を表示して確認します。)"),
        QStringLiteral(R"(.template <kind> [base]

kind:
  widget    Create a QWidget class with Q_%1OBJECT and moc include
  ui        Create only a .ui file
  qrc       Create only a .qrc file

base:
  Name source: widget class, ui form, or qrc resource.
  If omitted in an interactive terminal, icpp asks for it.
  Empty input, or non-interactive use, defaults by kind.

examples:
  .template widget counter
  .template ui settings
  .template qrc resources

Existing files are never overwritten. icpp shows the file list before creation.)"))
                   .arg(QString())
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::handleTemplateCommand(const QString& argument, const QString& historyEntry)
{
    QStringList parts = QProcess::splitCommand(argument);
    if (parts.isEmpty() || parts.first() == QStringLiteral("-h")
        || parts.first() == QStringLiteral("--help") || parts.first() == QStringLiteral("help")) {
        showTemplateHelp(historyEntry);
        return;
    }

    const QString kind = parts.takeFirst().toLower();
    if (kind != QStringLiteral("widget") && kind != QStringLiteral("ui")
        && kind != QStringLiteral("qrc")) {
        reportError(trMessage(QStringLiteral("不明な template kind です: %1。詳しくは .template -h"),
                              QStringLiteral("Unknown template kind: %1. Use .template -h for details."))
                        .arg(kind));
        return;
    }

    QString base = parts.join(QLatin1Char(' ')).trimmed();
    const QString defaultBase = defaultBaseForTemplateKind(kind);
    if (base.isEmpty() && shouldShowPrompt()) {
        standardOutput << trMessage(promptForTemplateKind(kind),
                                    englishPromptForTemplateKind(kind))
                              .arg(defaultBase)
                       << Qt::flush;
        const std::optional<QString> answer = readConsoleLine(QString());
        if (!answer.has_value()) {
            standardOutput << Qt::endl;
            return;
        }
        base = answer->trimmed();
    }
    if (base.isEmpty()) {
        base = defaultBase;
    }

    const QFileInfo baseInfo = templateBaseInfo(base, newFileNameMode);
    const QString className = classNameFromTemplateBase(base);
    if (!baseInfo.absoluteDir().exists()) {
        reportError(trMessage(QStringLiteral("%1: ディレクトリが見つかりません。"),
                              QStringLiteral("%1: Directory not found."))
                        .arg(baseInfo.absoluteDir().absolutePath()));
        return;
    }

    const QList<TemplateFile> templateFiles = filesForTemplate(kind, baseInfo, className);
    for (const TemplateFile& file : templateFiles) {
        const QFileInfo fileInfo(file.path);
        if (fileInfo.exists()) {
            reportError(trMessage(QStringLiteral("既存ファイルがあるため作成しません: %1"),
                                  QStringLiteral("File already exists; not creating templates: %1"))
                            .arg(fileInfo.filePath()));
            return;
        }
    }

    standardOutput << trMessage(QStringLiteral("作成するファイル:"),
                                QStringLiteral("Files to create:"))
                   << Qt::endl;
    for (const TemplateFile& file : templateFiles) {
        standardOutput << QStringLiteral("  %1").arg(file.path) << Qt::endl;
    }
    if (kind == QStringLiteral("widget")) {
        standardOutput << trMessage(QStringLiteral("class: %1"), QStringLiteral("class: %1"))
                              .arg(className)
                       << Qt::endl;
    } else if (kind == QStringLiteral("ui")) {
        standardOutput << trMessage(QStringLiteral("form class: %1"),
                                    QStringLiteral("form class: %1"))
                              .arg(className)
                       << Qt::endl;
    }

    if (!confirmYesNo(trMessage(QStringLiteral("続けますか? [Y/N] "),
                                QStringLiteral("Continue? [Y/N] ")))) {
        addHistoryEntry(historyEntry);
        return;
    }

    for (const TemplateFile& file : templateFiles) {
        if (!writeTextFile(file.path, file.sourceCode)) {
            return;
        }
        standardOutput << trMessage(QStringLiteral("作成しました: %1"),
                                    QStringLiteral("Created: %1"))
                              .arg(file.path)
                       << Qt::endl;
    }

    if (!quietMode) {
        standardOutput << trMessage(QStringLiteral("次の操作:"), QStringLiteral("Next steps:"))
                       << Qt::endl;
        showTemplateNextSteps(standardOutput, kind, baseInfo);
        showTemplateUsageHint(standardOutput,
                              kind,
                              baseInfo,
                              className,
                              trMessage(QStringLiteral("使い方の例:"),
                                        QStringLiteral("Usage example:")));
    }
    addHistoryEntry(historyEntry);
}

} // namespace icpp
