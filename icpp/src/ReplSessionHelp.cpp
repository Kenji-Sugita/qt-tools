#include "icpp/ReplSessionImpl.h"

#include <QTextStream>

namespace icpp {

namespace {

bool isHelpArgument(const QString& argument)
{
    const QString normalized = argument.trimmed().toLower();
    return normalized == QStringLiteral("-h") || normalized == QStringLiteral("--help")
           || normalized == QStringLiteral("help");
}

QString canonicalHelpCommand(const QString& commandName)
{
    QString normalized = commandName.trimmed().toLower();
    if (!normalized.startsWith(QLatin1Char('.'))) {
        normalized.prepend(QLatin1Char('.'));
    }

    if (normalized == QStringLiteral(".h") || normalized == QStringLiteral(".?")) {
        return QStringLiteral(".help");
    }
    if (normalized == QStringLiteral(".q")) {
        return QStringLiteral(".quit");
    }
    if (normalized == QStringLiteral(".st")) {
        return QStringLiteral(".status");
    }
    if (normalized == QStringLiteral(".b")) {
        return QStringLiteral(".buffer");
    }
    if (normalized == QStringLiteral(".language")) {
        return QStringLiteral(".lang");
    }
    if (normalized == QStringLiteral(".r")) {
        return QStringLiteral(".run");
    }
    if (normalized == QStringLiteral(".e")) {
        return QStringLiteral(".edit");
    }
    if (normalized == QStringLiteral(".sh")) {
        return QStringLiteral(".show");
    }
    if (normalized == QStringLiteral(".s")) {
        return QStringLiteral(".save");
    }
    if (normalized == QStringLiteral(".d")) {
        return QStringLiteral(".discard");
    }
    if (normalized == QStringLiteral(".de")) {
        return QStringLiteral(".designer");
    }
    if (normalized == QStringLiteral(".li")) {
        return QStringLiteral(".linguist");
    }
    if (normalized == QStringLiteral(".qtc")) {
        return QStringLiteral(".qrc");
    }
    if (normalized == QStringLiteral(".o")) {
        return QStringLiteral(".open");
    }
    if (normalized == QStringLiteral(".l")) {
        return QStringLiteral(".load");
    }
    if (normalized == QStringLiteral(".p")) {
        return QStringLiteral(".print");
    }
    if (normalized == QStringLiteral(".x")) {
        return QStringLiteral(".eval");
    }
    if (normalized == QStringLiteral(".a")) {
        return QStringLiteral(".append");
    }
    if (normalized == QStringLiteral(".pv")) {
        return QStringLiteral(".preview");
    }
    if (normalized == QStringLiteral(".new")) {
        return QStringLiteral(".template");
    }
    return normalized;
}

} // namespace

bool ReplSessionImpl::showCommandHelpIfRequested(const QString& commandName,
                                                 const QString& argument,
                                                 const QString& historyEntry)
{
    if (!isHelpArgument(argument)) {
        return false;
    }
    return showCommandHelp(commandName, historyEntry);
}

bool ReplSessionImpl::showCommandHelp(const QString& commandName, const QString& historyEntry)
{
    const QString canonicalCommand = canonicalHelpCommand(commandName);
    if (canonicalCommand == QStringLiteral(".template")) {
        showTemplateHelp(historyEntry);
        return true;
    }

    const QString text = commandHelpText(canonicalCommand);
    if (text.isEmpty()) {
        return false;
    }

    if (!showTextInPager(text)) {
        standardOutput << text << Qt::endl;
    }
    addHistoryEntry(historyEntry);
    return true;
}

QString ReplSessionImpl::commandHelpText(const QString& commandName) const
{
    if (helpLanguage == HelpLanguage::English) {
        if (commandName == QStringLiteral(".help")) {
            return QStringLiteral(R"(.help, .h, .?

Usage:
  .help
  .?
  .help <command>
  .command -h

Shows the command list. With a command name, shows detailed help for that command.)");
        }
        if (commandName == QStringLiteral(".args")) {
            return QStringLiteral(R"(.args

Usage:
  .args

Shows the active interpreter engine, program path, and arguments passed to cling/qtcling.)");
        }
        if (commandName == QStringLiteral(".status")) {
            return QStringLiteral(R"(.status, .st

Usage:
  .status

Shows session state: engine, help language, buffer append mode, autogen mode,
quiet mode, new file name mode, edit buffer state, save target, registered files,
include paths, touched files, Qt generated-file hints, and run_all availability.)");
        }
        if (commandName == QStringLiteral(".errors")) {
            return QStringLiteral(R"(.errors

Usage:
  .errors

Shows recent icpp command errors. C++ diagnostics from cling/qtcling are forwarded
directly and are not stored here.)");
        }
        if (commandName == QStringLiteral(".doctor")) {
            return QStringLiteral(R"(.doctor

Usage:
  .doctor

Checks the interpreter, icpp Qt, active Qt tools, current directory, generated
files, and registered file count. Use it when Qt-related commands do not behave
as expected.)");
        }
        if (commandName == QStringLiteral(".generated")) {
            return QStringLiteral(R"(.generated

Usage:
  .generated

Lists generated files in the current directory: moc_*.cpp, ui_*.h, qrc_*.cpp,
and *.qm. The output also shows whether source files are missing or possibly stale.)");
        }
        if (commandName == QStringLiteral(".examples")) {
            return QStringLiteral(R"(.examples

Usage:
  .examples

Shows short icpp workflow examples for expressions, files, Qt widgets, .ui, .qrc,
translations, and .inspect.)");
        }
        if (commandName == QStringLiteral(".where")) {
            return QStringLiteral(R"(.where

Usage:
  .where

Shows a compact location summary: current directory, save target, edit buffer state,
registered-file count, generated-file count, and autogen mode.)");
        }
        if (commandName == QStringLiteral(".runorder")) {
            return QStringLiteral(R"(.runorder

Usage:
  .runorder

Shows the exact file order used by .r/.run. Use .r edit to reorder it.)");
        }
        if (commandName == QStringLiteral(".qt")) {
            return QStringLiteral(R"(.qt

Usage:
  .qt

Shows a compact Qt status summary: icpp Qt, active Qt tools, .ui/.qrc/.ts counts,
Q_%1OBJECT source count, generated files, stale generated files, run_all,
QtUiTools, and autogen.)")
                       .arg(QString());
        }
        if (commandName == QStringLiteral(".clean")) {
            return QStringLiteral(R"(.clean

Usage:
  .clean

Shows generated files in the current directory and asks for confirmation before deleting them.
Targets include moc_*.cpp, ui_*.h, qrc_*.cpp, and *.qm.)");
        }
        if (commandName == QStringLiteral(".buffer")) {
            return QStringLiteral(R"(.buffer, .b

Usage:
  .buffer
  .buffer on
  .buffer off
  .b
  .b on
  .b off

Controls whether normal C++ input is appended to the edit buffer. The default is off.
.b without arguments toggles the mode. When enabled, the prompt shows +b.)");
        }
        if (commandName == QStringLiteral(".autogen")) {
            return QStringLiteral(R"(.autogen

Usage:
  .autogen
  .autogen on
  .autogen off

Controls automatic run_all execution before rerun when Qt generated files appear necessary.
The default is off. Use .gen when you want an explicit generation step.)");
        }
        if (commandName == QStringLiteral(".quiet")) {
            return QStringLiteral(R"(.quiet

Usage:
  .quiet
  .quiet on
  .quiet off

Shows or changes the persistent quiet setting.
When quiet is on, .template/.new still show confirmations and created files,
but suppress beginner next-step hints after creation.)");
        }
        if (commandName == QStringLiteral(".newname")) {
            return QStringLiteral(R"(.newname

Usage:
  .newname
  .newname lower
  .newname asis

Shows or changes how new files are named.
lower creates lowercase file names. asis preserves the typed name.
The setting is saved and affects .template/.new and new .designer forms.)");
        }
        if (commandName == QStringLiteral(".lang")) {
            return QStringLiteral(R"(.lang, .language

Usage:
  .lang
  .lang ja
  .lang en

Shows or changes the help language. The setting is persisted with QSettings.
Command-line --help-language and ICPP_HELP_LANGUAGE still override the stored setting at startup.)");
        }
        if (commandName == QStringLiteral(".run")) {
            return QStringLiteral(R"(.run, .r

Usage:
  .run
  .r
  .r edit

Restarts the interpreter, evaluates registered files in order, then evaluates the edit buffer.
.r edit opens the registered-file order in an editor before running.)");
        }
        if (commandName == QStringLiteral(".gen")) {
            return QStringLiteral(R"(.gen

Usage:
  .gen

Runs run_all in the current directory. If it succeeds, icpp restarts the interpreter
and evaluates registered files and the edit buffer.)");
        }
        if (commandName == QStringLiteral(".restart")) {
            return QStringLiteral(R"(.restart

Usage:
  .restart

Restarts the interpreter while keeping registered files, include paths, and the edit buffer.)");
        }
        if (commandName == QStringLiteral(".reset")) {
            return QStringLiteral(R"(.reset

Usage:
  .reset

Restarts the interpreter and clears registered files, include paths, touched files,
the edit buffer, and save target.)");
        }
        if (commandName == QStringLiteral(".edit")) {
            return QStringLiteral(R"(.edit, .e

Usage:
  .e
  .e <file>
  .e <number>

Without arguments, edits the temporary edit buffer. With a file, creates/registers it
if needed and edits that real file. With a number, edits that registered file.
After the editor closes, icpp reruns registered files and the edit buffer.)");
        }
        if (commandName == QStringLiteral(".show")) {
            return QStringLiteral(R"(.show, .sh

Usage:
  .show
  .show <start>
  .show <start>:<end>

Shows the edit buffer with line numbers. Range examples: .show 3, .show 3:8.)");
        }
        if (commandName == QStringLiteral(".save")) {
            return QStringLiteral(R"(.save, .s

Usage:
  .save
  .save <file>

Saves the edit buffer. When multiple real files are in use, omitting the file name
can be ambiguous and icpp asks for confirmation.)");
        }
        if (commandName == QStringLiteral(".discard")) {
            return QStringLiteral(R"(.discard, .d

Usage:
  .discard

Clears the edit buffer. If the buffer has unsaved changes, icpp asks for confirmation.)");
        }
        if (commandName == QStringLiteral(".designer")) {
            return QStringLiteral(R"(.designer, .de

Usage:
  .designer <form|file.ui>
  .de <form|file.ui>

Creates or edits a .ui file with Qt Designer.
Without a .ui suffix, the argument is treated as a form name.
New file casing follows .newname lower/asis.
New file creation shows the file name and form class before asking for confirmation.
After saving, run .gen to update ui_*.h, or enable .autogen on.)");
        }
        if (commandName == QStringLiteral(".linguist")) {
            return QStringLiteral(R"(.linguist, .li

Usage:
  .linguist <file.ts>
  .li <file.ts>

Opens a .ts file with Qt Linguist. After editing, run .! lrelease <file.ts>
when you need to update the .qm file.)");
        }
        if (commandName == QStringLiteral(".qrc")) {
            return QStringLiteral(R"(.qrc, .qtc

Usage:
  .qrc <file.qrc>
  .qrc text <file.qrc>
  .qrc creator <file.qrc>
  .qtc <file.qrc>

Edits a Qt resource file. The text mode uses the configured editor. The creator
mode opens Qt Creator. Run .gen after changing resources.)");
        }
        if (commandName == QStringLiteral(".open")) {
            return QStringLiteral(R"(.open, .o

Usage:
  .open <file>
  .o <file>

Replaces the edit buffer with a file without evaluating it.)");
        }
        if (commandName == QStringLiteral(".load")) {
            return QStringLiteral(R"(.load, .l

Usage:
  .load <file>
  .l <file>

Reads a file, sends it to the interpreter, and appends it to the edit buffer.)");
        }
        if (commandName == QStringLiteral(".paste")) {
            return QStringLiteral(R"(.paste

Usage:
  .paste

Appends clipboard text to the edit buffer using an external clipboard command.)");
        }
        if (commandName == QStringLiteral(".copy")) {
            return QStringLiteral(R"(.copy

Usage:
  .copy

Copies the edit buffer to the clipboard using an external clipboard command.)");
        }
        if (commandName == QStringLiteral(".add")) {
            return QStringLiteral(R"(.add

Usage:
  .add <file>

Registers a source file evaluated by .run/.r. Register implementation files normally;
headers are usually included from those implementation files. .h/.hpp files are not registered.)");
        }
        if (commandName == QStringLiteral(".files")) {
            return QStringLiteral(R"(.files

Usage:
  .files

Shows registered files in the order used by .run/.r.)");
        }
        if (commandName == QStringLiteral(".drop")) {
            return QStringLiteral(R"(.drop

Usage:
  .drop <file>
  .drop <number>

Removes a file from the registered-file list.)");
        }
        if (commandName == QStringLiteral(".clearfiles")) {
            return QStringLiteral(R"(.clearfiles

Usage:
  .clearfiles

Clears all registered files. If multiple files are registered, icpp asks for confirmation.)");
        }
        if (commandName == QStringLiteral(".print")) {
            return QStringLiteral(R"(.print, .p

Usage:
  .print <expr>
  .p <expr>

Prints an expression without appending it to the edit buffer.)");
        }
        if (commandName == QStringLiteral(".ptype")) {
            return QStringLiteral(R"(.ptype

Usage:
  .ptype <expr>

Prints the C++/Qt type of an expression.)");
        }
        if (commandName == QStringLiteral(".defs")) {
            return QStringLiteral(R"(.defs

Usage:
  .defs

Shows definitions found in registered files and the edit buffer.)");
        }
        if (commandName == QStringLiteral(".eval")) {
            return QStringLiteral(R"(.eval, .x

Usage:
  .x <code>
  .eval <code>

Evaluates C++ code without appending it to the edit buffer. Use this for calls,
temporary variables, and checks that should not rerun on .r.)");
        }
        if (commandName == QStringLiteral(".append")) {
            return QStringLiteral(R"(.append, .a

Usage:
  .a <code>
  .append <code>

Evaluates C++ code and appends it to the edit buffer. This is useful when .b is off
but you want to keep a particular line for future .r runs.)");
        }
        if (commandName == QStringLiteral(".!")) {
            return QStringLiteral(R"(.!

Usage:
  .! <command>
  .!command

Runs an external shell command. Both .! pwd and .!pwd are accepted.)");
        }
        if (commandName == QStringLiteral(".i")) {
            return QStringLiteral(R"(.i

Usage:
  .i
  .i <path>
  .i edit

Shows include paths, adds one by sending #pragma cling add_include_path("path"),
or opens the include path list in an editor.)");
        }
        if (commandName == QStringLiteral(".include")) {
            return QStringLiteral(R"(.include

Usage:
  .include <header>

Sends #include <header> to the interpreter.)");
        }
        if (commandName == QStringLiteral(".pragma")) {
            return QStringLiteral(R"(.pragma

Usage:
  .pragma <text>

Sends #pragma cling <text> to the interpreter.)");
        }
        if (commandName == QStringLiteral(".loadlib")) {
            return QStringLiteral(R"(.loadlib

Usage:
  .loadlib <path>

Sends #pragma cling load("path") to the interpreter.)");
        }
        if (commandName == QStringLiteral(".uiinfo")) {
            return QStringLiteral(R"(.uiinfo

Usage:
  .uiinfo <file.ui>

Reads a .ui file directly and lists widgets, layouts, actions, and spacers.
No files are generated.)");
        }
        if (commandName == QStringLiteral(".preview")) {
            return QStringLiteral(R"(.preview, .pv

Usage:
  .preview <file.ui>
  .pv <file.ui>

Previews a .ui file directly with QUiLoader. No ui_*.h file is generated.)");
        }
        if (commandName == QStringLiteral(".inspect")) {
            return QStringLiteral(R"(.inspect

Usage:
  .inspect

Opens the icpp inspector for visible top-level QWidget objects. Requires qtcling
and the PropertyEditor support library. On Linux, set ICPP_PROPERTY_EDITOR_LIB
and ICPP_PROPERTY_EDITOR_INCLUDE.)");
        }
        if (commandName == QStringLiteral(".widgets")) {
            return QStringLiteral(R"(.widgets

Usage:
  .widgets
  .widgets all

Shows top-level QWidget objects. The all argument also includes hidden top-level widgets.)");
        }
        if (commandName == QStringLiteral(".closeall")) {
            return QStringLiteral(R"(.closeall

Usage:
  .closeall

Closes user-created visible top-level QWidget objects. icpp internal windows are ignored.)");
        }
        if (commandName == QStringLiteral(".quit")) {
            return QStringLiteral(R"(.quit, .q

Usage:
  .quit
  .q

Quits icpp. If the edit buffer has unsaved changes, icpp asks for confirmation.)");
        }
        return {};
    }

    if (commandName == QStringLiteral(".help")) {
        return QStringLiteral(R"(.help, .h, .?

使い方:
  .help
  .?
  .help <command>
  .command -h

コマンド一覧を表示します。コマンド名を指定すると、そのコマンドの詳細ヘルプを表示します。)");
    }
    if (commandName == QStringLiteral(".args")) {
        return QStringLiteral(R"(.args

使い方:
  .args

使用中の interpreter engine、プログラムパス、cling/qtcling に渡している引数を表示します。)");
    }
    if (commandName == QStringLiteral(".status")) {
        return QStringLiteral(R"(.status, .st

使い方:
  .status

engine、ヘルプ言語、入力バッファ追加、autogen、quiet、新規ファイル名、
編集バッファ、保存先、登録ファイル、include path、操作したファイル、
Qt 生成物の必要性、run_all を表示します。)");
    }
    if (commandName == QStringLiteral(".errors")) {
        return QStringLiteral(R"(.errors

使い方:
  .errors

直近の icpp コマンドエラーを表示します。cling/qtcling の C++ 診断はそのまま転送され、
ここには保存されません。)");
    }
    if (commandName == QStringLiteral(".doctor")) {
        return QStringLiteral(R"(.doctor

使い方:
  .doctor

interpreter、icpp Qt、active Qt tools、現在ディレクトリ、生成物、登録ファイル数を確認します。
Qt 関連コマンドが期待通り動かないときに使います。)");
    }
    if (commandName == QStringLiteral(".generated")) {
        return QStringLiteral(R"(.generated

使い方:
  .generated

現在ディレクトリの生成物を表示します。対象は moc_*.cpp、ui_*.h、qrc_*.cpp、*.qm です。
元ファイル不明や古い可能性も表示します。)");
    }
    if (commandName == QStringLiteral(".examples")) {
        return QStringLiteral(R"(.examples

使い方:
  .examples

式、実ファイル、Qt widget、.ui、.qrc、翻訳、.inspect の短い操作例を表示します。)");
    }
    if (commandName == QStringLiteral(".where")) {
        return QStringLiteral(R"(.where

使い方:
  .where

カレントディレクトリ、保存先、編集バッファ、登録ファイル数、生成物数、
autogen を短く表示します。)");
    }
    if (commandName == QStringLiteral(".runorder")) {
        return QStringLiteral(R"(.runorder

使い方:
  .runorder

.r/.run で評価するファイル順を表示します。順番を変えるには .r edit を使います。)");
    }
    if (commandName == QStringLiteral(".qt")) {
        return QStringLiteral(R"(.qt

使い方:
  .qt

icpp Qt、active Qt tools、.ui/.qrc/.ts の数、Q_%1OBJECT を含む source 数、
生成物、古い生成物、run_all、QtUiTools、autogen を短く表示します。)")
                   .arg(QString());
    }
    if (commandName == QStringLiteral(".clean")) {
        return QStringLiteral(R"(.clean

使い方:
  .clean

現在ディレクトリの生成物を表示し、確認してから削除します。
対象は moc_*.cpp、ui_*.h、qrc_*.cpp、*.qm です。)");
    }
    if (commandName == QStringLiteral(".buffer")) {
        return QStringLiteral(R"(.buffer, .b

使い方:
  .buffer
  .buffer on
  .buffer off
  .b
  .b on
  .b off

通常入力を編集バッファに追加するかを制御します。既定は off です。
.b を引数なしで実行すると on/off を切り替えます。on の間はプロンプトに +b が出ます。)");
    }
    if (commandName == QStringLiteral(".autogen")) {
        return QStringLiteral(R"(.autogen

使い方:
  .autogen
  .autogen on
  .autogen off

Qt 生成物が必要そうなときに、再実行前に run_all を自動実行するかを制御します。
既定は off です。明示的に生成したい場合は .gen を使います。)");
    }
    if (commandName == QStringLiteral(".quiet")) {
        return QStringLiteral(R"(.quiet

使い方:
  .quiet
  .quiet on
  .quiet off

保存される quiet 設定を表示・変更します。
quiet が on のときも、.template/.new の確認と作成結果は表示します。
作成後の初心者向けの次の操作ヒントだけを抑制します。)");
    }
    if (commandName == QStringLiteral(".newname")) {
        return QStringLiteral(R"(.newname

使い方:
  .newname
  .newname lower
  .newname asis

新規ファイル名の大文字小文字設定を表示・変更します。
lower は小文字のファイル名を作ります。asis は入力した名前をそのまま使います。
設定は保存され、.template/.new と .designer の新規 form 作成に効きます。)");
    }
    if (commandName == QStringLiteral(".lang")) {
        return QStringLiteral(R"(.lang, .language

使い方:
  .lang
  .lang ja
  .lang en

ヘルプ表示と icpp の表示メッセージの言語を表示・変更します。
設定は QSettings に保存されます。
起動時に --help-language または ICPP_HELP_LANGUAGE を指定した場合は、それらが保存値より優先されます。)");
    }
    if (commandName == QStringLiteral(".run")) {
        return QStringLiteral(R"(.run, .r

使い方:
  .run
  .r
  .r edit

interpreter を再起動し、登録ファイルを順番に評価してから編集バッファを評価します。
.r edit は実行前に登録ファイルの順番を外部エディタで変更します。)");
    }
    if (commandName == QStringLiteral(".gen")) {
        return QStringLiteral(R"(.gen

使い方:
  .gen

現在ディレクトリで run_all を実行します。成功したら interpreter を再起動し、
登録ファイルと編集バッファを評価します。)");
    }
    if (commandName == QStringLiteral(".restart")) {
        return QStringLiteral(R"(.restart

使い方:
  .restart

interpreter を再起動します。登録ファイル、include path、編集バッファは残します。)");
    }
    if (commandName == QStringLiteral(".reset")) {
        return QStringLiteral(R"(.reset

使い方:
  .reset

interpreter を再起動し、登録ファイル、include path、操作したファイル、
編集バッファ、保存先を消します。)");
    }
    if (commandName == QStringLiteral(".edit")) {
        return QStringLiteral(R"(.edit, .e

使い方:
  .e
  .e <file>
  .e <number>

引数なしでは一時的な編集バッファを編集します。file 指定では必要なら作成・登録し、
その実ファイルを編集します。number 指定では登録ファイルを編集します。
エディタ終了後、登録ファイルと編集バッファを再実行します。)");
    }
    if (commandName == QStringLiteral(".show")) {
        return QStringLiteral(R"(.show, .sh

使い方:
  .show
  .show <start>
  .show <start>:<end>

編集バッファを行番号付きで表示します。範囲指定例: .show 3、.show 3:8。)");
    }
    if (commandName == QStringLiteral(".save")) {
        return QStringLiteral(R"(.save, .s

使い方:
  .save
  .save <file>

編集バッファを保存します。複数の実ファイルを扱っているときに file を省略すると
保存先が曖昧になるため、確認します。)");
    }
    if (commandName == QStringLiteral(".discard")) {
        return QStringLiteral(R"(.discard, .d

使い方:
  .discard

編集バッファを消します。未保存の変更がある場合は確認します。)");
    }
    if (commandName == QStringLiteral(".designer")) {
        return QStringLiteral(R"(.designer, .de

使い方:
  .designer <form|file.ui>
  .de <form|file.ui>

.ui ファイルを作成または Qt Designer で編集します。
.ui 拡張子がない場合は form 名として扱います。
新規ファイル名の大文字小文字は .newname lower/asis に従います。
新規作成時はファイル名と form class を表示して確認します。
保存後は .gen で ui_*.h を更新します。
.autogen on の場合は編集後に自動生成を試みます。)");
    }
    if (commandName == QStringLiteral(".linguist")) {
        return QStringLiteral(R"(.linguist, .li

使い方:
  .linguist <file.ts>
  .li <file.ts>

.ts ファイルを Qt Linguist で開きます。編集後に .qm を更新する場合は
.! lrelease <file.ts> を実行します。)");
    }
    if (commandName == QStringLiteral(".qrc")) {
        return QStringLiteral(R"(.qrc, .qtc

使い方:
  .qrc <file.qrc>
  .qrc text <file.qrc>
  .qrc creator <file.qrc>
  .qtc <file.qrc>

Qt resource file を編集します。text は設定済みエディタ、creator は Qt Creator を使います。
resource を変更した後は .gen を実行します。)");
    }
    if (commandName == QStringLiteral(".open")) {
        return QStringLiteral(R"(.open, .o

使い方:
  .open <file>
  .o <file>

ファイル内容で編集バッファを置き換えます。実行はしません。)");
    }
    if (commandName == QStringLiteral(".load")) {
        return QStringLiteral(R"(.load, .l

使い方:
  .load <file>
  .l <file>

ファイルを読み込み、interpreter に送り、編集バッファにも追加します。)");
    }
    if (commandName == QStringLiteral(".paste")) {
        return QStringLiteral(R"(.paste

使い方:
  .paste

外部クリップボードコマンドを使って、クリップボードのテキストを編集バッファへ追加します。)");
    }
    if (commandName == QStringLiteral(".copy")) {
        return QStringLiteral(R"(.copy

使い方:
  .copy

外部クリップボードコマンドを使って、編集バッファをクリップボードへコピーします。)");
    }
    if (commandName == QStringLiteral(".add")) {
        return QStringLiteral(R"(.add

使い方:
  .add <file>

.run/.r で評価する source file を登録します。通常は実装ファイルを登録し、
header はその実装ファイルから include します。.h / .hpp は登録しません。)");
    }
    if (commandName == QStringLiteral(".files")) {
        return QStringLiteral(R"(.files

使い方:
  .files

.run/.r で評価する登録ファイルを順番付きで表示します。)");
    }
    if (commandName == QStringLiteral(".drop")) {
        return QStringLiteral(R"(.drop

使い方:
  .drop <file>
  .drop <number>

登録ファイル一覧からファイルを外します。)");
    }
    if (commandName == QStringLiteral(".clearfiles")) {
        return QStringLiteral(R"(.clearfiles

使い方:
  .clearfiles

登録ファイルを全て消します。複数ファイルが登録されている場合は確認します。)");
    }
    if (commandName == QStringLiteral(".print")) {
        return QStringLiteral(R"(.print, .p

使い方:
  .print <expr>
  .p <expr>

式を表示します。編集バッファには追加しません。)");
    }
    if (commandName == QStringLiteral(".ptype")) {
        return QStringLiteral(R"(.ptype

使い方:
  .ptype <expr>

式の C++/Qt 型を表示します。)");
    }
    if (commandName == QStringLiteral(".defs")) {
        return QStringLiteral(R"(.defs

使い方:
  .defs

登録ファイルと編集バッファから見つかった定義を表示します。)");
    }
    if (commandName == QStringLiteral(".eval")) {
        return QStringLiteral(R"(.eval, .x

使い方:
  .x <code>
  .eval <code>

C++ コードを評価します。編集バッファには追加しません。
関数呼び出し、一時変数、.r で再実行したくない確認に使います。)");
    }
    if (commandName == QStringLiteral(".append")) {
        return QStringLiteral(R"(.append, .a

使い方:
  .a <code>
  .append <code>

C++ コードを評価し、編集バッファにも追加します。.b off のまま、
必要な行だけ次回 .r 用に残したいときに使います。)");
    }
    if (commandName == QStringLiteral(".!")) {
        return QStringLiteral(R"(.!

使い方:
  .! <command>
  .!command

外部 shell コマンドを実行します。.! pwd と .!pwd のどちらも使えます。)");
    }
    if (commandName == QStringLiteral(".i")) {
        return QStringLiteral(R"(.i

使い方:
  .i
  .i <path>
  .i edit

インクルードパスを表示します。パスを指定すると #pragma cling add_include_path("path")
を interpreter に送ります。.i edit はインクルードパス一覧をエディターで編集します。)");
    }
    if (commandName == QStringLiteral(".include")) {
        return QStringLiteral(R"(.include

使い方:
  .include <header>

#include <header> を interpreter に送ります。)");
    }
    if (commandName == QStringLiteral(".pragma")) {
        return QStringLiteral(R"(.pragma

使い方:
  .pragma <text>

#pragma cling <text> を interpreter に送ります。)");
    }
    if (commandName == QStringLiteral(".loadlib")) {
        return QStringLiteral(R"(.loadlib

使い方:
  .loadlib <path>

#pragma cling load("path") を interpreter に送ります。)");
    }
    if (commandName == QStringLiteral(".uiinfo")) {
        return QStringLiteral(R"(.uiinfo

使い方:
  .uiinfo <file.ui>

.ui ファイルを直接読み、widget、layout、action、spacer を表示します。
ファイル生成は行いません。)");
    }
    if (commandName == QStringLiteral(".preview")) {
        return QStringLiteral(R"(.preview, .pv

使い方:
  .preview <file.ui>
  .pv <file.ui>

.ui ファイルを QUiLoader で直接プレビューします。ui_*.h は生成しません。)");
    }
    if (commandName == QStringLiteral(".inspect")) {
        return QStringLiteral(R"(.inspect

使い方:
  .inspect

表示中の top-level QWidget を icpp inspector で調べます。
qtcling と PropertyEditor support library が必要です。
Linux では ICPP_PROPERTY_EDITOR_LIB と ICPP_PROPERTY_EDITOR_INCLUDE を指定します。)");
    }
    if (commandName == QStringLiteral(".widgets")) {
        return QStringLiteral(R"(.widgets

使い方:
  .widgets
  .widgets all

top-level QWidget を表示します。all を付けると非表示の top-level widget も含めます。)");
    }
    if (commandName == QStringLiteral(".closeall")) {
        return QStringLiteral(R"(.closeall

使い方:
  .closeall

利用者が作成した表示中の top-level QWidget を閉じます。icpp 内部 window は対象外です。)");
    }
    if (commandName == QStringLiteral(".quit")) {
        return QStringLiteral(R"(.quit, .q

使い方:
  .quit
  .q

icpp を終了します。編集バッファに未保存の変更があれば確認します。)");
    }
    return {};
}

} // namespace icpp
