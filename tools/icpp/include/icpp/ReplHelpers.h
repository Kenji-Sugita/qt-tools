#pragma once

#include "icpp/repl.h"

#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QDir>
#include <QString>
#include <QStringList>

#include <optional>

class QTextStream;

namespace icpp {

inline constexpr auto kDefaultEditor = "vim";
inline constexpr auto kDefaultClingPath = "/usr/local/src/cling/build/bin/cling";
inline constexpr auto kDefaultQtClingPath = "/usr/local/src/cling/bin/qtcling";

inline constexpr auto kUsageTextEnglish = R"(Session
  .help, .h, .?                 Show this help
                                Use .command -h for detailed command help
  .args                         Show the active interpreter program and arguments
  .status, .st                  Show the current session status
  .errors                       Show recent icpp command errors
  .doctor                       Check interpreter, Qt tools, and current directory
  .generated                    Show generated files in the current directory
  .examples                     Show short workflow examples
  .where                        Show a compact location and buffer summary
  .runorder                     Show registered file evaluation order
  .qt                           Show compact Qt generated-file status
  .clean                        Delete generated files after confirmation
  .buffer, .b [on|off]          Show, set, or toggle normal input buffer append
                                Default is off
  .autogen [on|off]             Show or set Qt generated file autogen
  .quiet [on|off]               Show or set persistent quiet hints
  .newname [lower|asis]         Show or set new file name casing
  .lang [ja|en]                 Show or set persistent help language
  .quit, .q                     Quit icpp

Run and reset
  .run, .r [edit]               Restart and evaluate registered files, then buffer
                                Use edit to reorder registered files first
  .gen                          Run run_all, then restart and evaluate files and buffer
  .restart                      Restart the interpreter and keep files and buffer
  .reset                        Restart the interpreter and clear files and buffer

Edit buffer
  .edit, .e [file|number]       Edit the buffer or a registered file, then run
  .show, .sh [range]            Show the buffer with line numbers
  .save, .s [file]              Save the buffer
  .discard, .d                  Clear the buffer
  .designer, .de <form|file.ui> Create or edit a .ui file with Qt Designer
  .linguist, .li <file.ts>      Open a .ts file with Qt Linguist
  .qrc [text|creator] <file.qrc> Edit a .qrc file
  .qtc <file.qrc>               Edit a .qrc file with Qt Creator
  .template <kind> [base]       Create starter files. Use .template -h for details
  .new <kind> [base]            Alias for .template
  .open, .o <file>              Replace the buffer with a file without evaluating it
  .load, .l <file>              Load, evaluate, and append a file to the buffer
  .paste                        Append clipboard text to the buffer
  .copy                         Copy the buffer to the clipboard

Registered files
  .add <file>                   Add a file evaluated by .run/.r
  .files                        Show files evaluated by .run/.r
  .drop <file|number>           Remove a registered file
  .clearfiles                   Clear registered files

Inspect values
  .print, .p <expr>             Print an expression without changing the buffer
  .ptype <expr>                 Print the C++/Qt type of an expression
  .defs                         Show definitions in registered files and buffer
  .eval, .x <code>              Evaluate C++ code without changing the buffer
  .append, .a <code>            Evaluate C++ code and append it to the buffer

Interpreter
  .! <command>                  Run an external shell command
  .i [path|edit]                Show, add, or edit include paths
  .include <header>             Send #include <header>
  .pragma <text>                Send #pragma cling <text>
  .loadlib <path>               Send #pragma cling load("path")

Qt widgets
  .uiinfo <file.ui>             Show widgets, layouts, and actions in a .ui file
  .preview, .pv <file.ui>       Preview a .ui file directly with QUiLoader
  .inspect                      Inspect top-level widgets with PropertyEditor
  .widgets [all]                Show top-level QWidget objects
  .closeall                     Close top-level QWidget objects

Range syntax for .show: start or start:end
Examples: .show 3   .show 3:8)";

inline constexpr auto kUsageTextJapanese = R"(セッション
  .help, .h, .?                 このヘルプを表示します
                                詳細は .command -h を使います
  .args                         使用中の interpreter と引数を表示します
  .status, .st                  セッション状態を表示します
  .errors                       直近の icpp コマンドエラーを表示します
  .doctor                       interpreter、Qt ツール、現在ディレクトリを確認します
  .generated                    現在ディレクトリの生成物一覧を表示します
  .examples                     短い操作例を表示します
  .where                        現在位置と編集状態を短く表示します
  .runorder                     登録ファイルの評価順を表示します
  .qt                           Qt 生成物まわりの状態を短く表示します
  .clean                        生成物を確認してから削除します
  .buffer, .b [on|off]          通常入力を編集バッファに追加するか表示・切り替えします
                                既定は off です
  .autogen [on|off]             Qt 生成物の自動更新を表示・切り替えします
  .quiet [on|off]               ヒント表示を減らす quiet 設定を表示・切り替えします
  .newname [lower|asis]         新規ファイル名の大文字小文字設定を表示・切り替えします
  .lang [ja|en]                 ヘルプ言語を表示・切り替えします
  .quit, .q                     icpp を終了します

実行とリセット
  .run, .r [edit]               interpreter を再起動し、登録ファイルと編集バッファを評価します
                                edit 付きでは先に登録ファイルの順番をエディタで変更します
  .gen                          run_all を実行してから .run と同じ再評価をします
  .restart                      interpreter を再起動します。登録ファイルと編集バッファは残します
  .reset                        interpreter を再起動し、登録ファイルと編集バッファを消します

編集バッファ
  .edit, .e [file|number]       編集バッファまたは登録ファイルを編集し、閉じた後に実行します
  .show, .sh [range]            編集バッファを行番号付きで表示します
  .save, .s [file]              編集バッファを保存します
  .discard, .d                  編集バッファを消します
  .designer, .de <form|file.ui> .ui ファイルを作成または Qt Designer で編集します
  .linguist, .li <file.ts>      .ts ファイルを Qt Linguist で編集します
  .qrc [text|creator] <file.qrc> .qrc ファイルを編集します
  .qtc <file.qrc>               .qrc ファイルを Qt Creator で編集します
  .template <kind> [base]       ひな形ファイルを作成します。詳しくは .template -h
  .new <kind> [base]            .template の別名です
  .open, .o <file>              ファイル内容で編集バッファを置き換えます。実行はしません
  .load, .l <file>              ファイルを読み込んで実行し、編集バッファにも追加します
  .paste                        クリップボードのテキストを編集バッファへ追加します
  .copy                         編集バッファをクリップボードへコピーします

登録ファイル
  .add <file>                   .run/.r で評価するファイルを登録します
  .files                        登録ファイル一覧を表示します
  .drop <file|number>           登録ファイルを外します
  .clearfiles                   登録ファイルを全て外します

値の確認
  .print, .p <expr>             式を表示します。編集バッファには追加しません
  .ptype <expr>                 式の C++/Qt 型を表示します
  .defs                         登録ファイルと編集バッファの定義一覧を表示します
  .eval, .x <code>              C++ コードを実行します。編集バッファには追加しません
  .append, .a <code>            C++ コードを実行し、編集バッファにも追加します

interpreter
  .! <command>                  外部 shell コマンドを実行します
  .i [path|edit]                インクルードパスを表示・追加・編集します
  .include <header>             #include <header> を送ります
  .pragma <text>                #pragma cling <text> を送ります
  .loadlib <path>               #pragma cling load("path") を送ります

Qt widget
  .uiinfo <file.ui>             .ui ファイル内の widget、layout、action を表示します
  .preview, .pv <file.ui>       .ui ファイルを QUiLoader で直接プレビューします
  .inspect                      PropertyEditor で top-level QWidget を調べます
  .widgets [all]                top-level QWidget 一覧を表示します
  .closeall                     top-level QWidget をまとめて閉じます

.show の範囲指定: start または start:end
例: .show 3   .show 3:8)";


struct ParsedCommand {
    QString commandName;
    QString argument;
};

struct ShowRange {
    int startLine = 1;
    int endLine = 1;
};

struct ClipboardCommand {
    QString program;
    QStringList arguments;
};

struct SourceDefinition {
    int lineNumber = 0;
    QString description;
};

using MemberAccessMap = QHash<QString, QString>;

struct UiObjectInfo {
    QString typeName;
    QString objectName;
};

QTextStream& standardOutput();
QTextStream& standardError();
HelpLanguage& activeHelpLanguage();
QString trGlobalMessage(const QString& japanese, const QString& english);

bool shouldShowPrompt();
bool shouldUsePager();
QString pagerCommand();
std::optional<QString> readConsoleLine(const QString& promptText);
ParsedCommand parseCommand(const QString& trimmedLine);
QString normalizeSourceCode(const QString& sourceCode);
QString joinSourceCode(const QStringList& blocks);
std::optional<QString> readTextFile(const QString& filePath);
bool writeTextFile(const QString& filePath, const QString& sourceCode);
QString quoteForPragma(const QString& path);
QString quoteForCppString(const QString& text);
QString qtUiToolsLibraryPath();
QString propertyEditorLibraryPath();
QString propertyEditorIncludePath();
QString configuredPath(const char* environmentVariableName, const char* fallbackPath);
bool shouldAutoRegisterEditedFile(const QFileInfo& fileInfo);
bool isAddableSourceFile(const QFileInfo& fileInfo);
QStringList missingMocIncludes(const QString& sourceCode, const QDir& baseDirectory);
int braceDeltaOutsideQuotedText(const QString& line);
MemberAccessMap memberAccessMapForSource(const QString& sourceCode, const QDir& baseDirectory);
QList<SourceDefinition> findSourceDefinitions(const QString& sourceCode);
QString definitionDescriptionWithAccess(const SourceDefinition& definition,
                                       const MemberAccessMap& accessMap);
std::optional<ShowRange> parseShowRange(const QString& rangeArgument, int availableLineCount);
QString findProgramInPath(const QString& programName);
std::optional<ClipboardCommand> clipboardCommandFromEnvironment(const char* environmentVariable);
std::optional<ClipboardCommand> findClipboardReadCommand();
std::optional<ClipboardCommand> findClipboardWriteCommand();
QString resolveInterpreterProgramImpl(Engine engine,
                                      const QString& commandLinePath,
                                      HelpLanguage helpLanguage,
                                      QTextStream& errorStream);

} // namespace icpp
