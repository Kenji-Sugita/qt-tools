#include "icpp/Application.h"
#include "icpp/McpHttpServer.h"
#include "icpp/repl.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QSettings>
#include <QTextStream>

namespace {

QTextStream& standardOutput()
{
    static QTextStream stream(stdout);
    return stream;
}

QTextStream& standardError()
{
    static QTextStream stream(stderr);
    return stream;
}

QStringList parseRepeatedOptionValues(const QCommandLineParser& parser,
                                      const QCommandLineOption& option)
{
    return parser.values(option);
}

QString storedHelpLanguageName()
{
    return QSettings(QStringLiteral("icpp"), QStringLiteral("icpp"))
        .value(QStringLiteral("helpLanguage"))
        .toString();
}

icpp::HelpLanguage initialHelpLanguage(int argc, char* argv[])
{
    QString languageName;
    for (int index = 1; index < argc; ++index) {
        const QString argument = QString::fromLocal8Bit(argv[index]);
        if (argument == QStringLiteral("--help-language") && index + 1 < argc) {
            languageName = QString::fromLocal8Bit(argv[index + 1]);
            ++index;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--help-language="))) {
            languageName = argument.mid(QStringLiteral("--help-language=").size());
        }
    }
    if (languageName.isEmpty()) {
        languageName = qEnvironmentVariable("ICPP_HELP_LANGUAGE");
    }
    if (languageName.isEmpty()) {
        languageName = storedHelpLanguageName();
    }

    const std::optional<icpp::HelpLanguage> parsedLanguage = icpp::parseHelpLanguageName(languageName);
    return parsedLanguage.value_or(icpp::HelpLanguage::Japanese);
}

QString trMessage(icpp::HelpLanguage language, const QString& japanese, const QString& english)
{
    return language == icpp::HelpLanguage::Japanese ? japanese : english;
}

void showCommandLineHelp(icpp::HelpLanguage language)
{
    QTextStream& output = standardOutput();
    output << trMessage(language,
                        QStringLiteral("使い方: icpp [options]\n"),
                        QStringLiteral("Usage: icpp [options]\n"));
    output << trMessage(language,
                        QStringLiteral("cling / qtcling を使いやすくする対話型 C++ REPL ラッパー。\n\n"),
                        QStringLiteral("Interactive C++ REPL wrapper for cling and qtcling.\n\n"));
    output << trMessage(language, QStringLiteral("オプション:\n"), QStringLiteral("Options:\n"));
    output << QStringLiteral("  -h, --help                  ")
           << trMessage(language,
                        QStringLiteral("ヘルプを表示します。"),
                        QStringLiteral("Show this help."))
           << Qt::endl;
    output << QStringLiteral("  -v, --version               ")
           << trMessage(language,
                        QStringLiteral("バージョンを表示します。"),
                        QStringLiteral("Show version information."))
           << Qt::endl;
    output << QStringLiteral("  --engine <engine>           ")
           << trMessage(language,
                        QStringLiteral("interpreter engine を選択します: cling または qtcling。"),
                        QStringLiteral("Select the interpreter engine: cling or qtcling."))
           << Qt::endl;
    output << QStringLiteral("  --cling <path>              ")
           << trMessage(language,
                        QStringLiteral("cling 実行ファイルのパス。"),
                        QStringLiteral("Path to the cling executable."))
           << Qt::endl;
    output << QStringLiteral("  --qtcling <path>            ")
           << trMessage(language,
                        QStringLiteral("qtcling 実行ファイルのパス。"),
                        QStringLiteral("Path to the qtcling executable."))
           << Qt::endl;
    output << QStringLiteral("  --interpreter-arg <arg>     ")
           << trMessage(language,
                        QStringLiteral("選択中の interpreter に渡す追加引数。"),
                        QStringLiteral("Additional argument passed to the selected interpreter."))
           << Qt::endl;
    output << QStringLiteral("  --cling-arg <arg>           ")
           << trMessage(language,
                        QStringLiteral("--engine cling のときだけ cling に渡す追加引数。"),
                        QStringLiteral("Additional argument passed to cling when --engine cling is selected."))
           << Qt::endl;
    output << QStringLiteral("  --qtcling-arg <arg>         ")
           << trMessage(language,
                        QStringLiteral("--engine qtcling のときだけ qtcling に渡す追加引数。"),
                        QStringLiteral("Additional argument passed to qtcling when --engine qtcling is selected."))
           << Qt::endl;
    output << QStringLiteral("  --help-language <language>  ")
           << trMessage(language,
                        QStringLiteral("icpp のヘルプと表示メッセージの言語: ja または en。"),
                        QStringLiteral("Language for icpp help and messages: ja or en."))
           << Qt::endl;
    output << QStringLiteral("  --mcp-http                 ")
           << trMessage(language,
                        QStringLiteral("ローカル Streamable HTTP MCP Server として起動します。"),
                        QStringLiteral("Run as a local Streamable HTTP MCP server."))
           << Qt::endl;
    output << QStringLiteral("  --mcp-port <port>          ")
           << trMessage(language,
                        QStringLiteral("MCP HTTP port。既定値は 8766。"),
                        QStringLiteral("MCP HTTP port. The default is 8766."))
           << Qt::endl;
    output << QStringLiteral("  --mcp-allow-execution      ")
           << trMessage(language,
                        QStringLiteral("MCP からの任意コード実行を明示的に許可します。"),
                        QStringLiteral("Explicitly allow arbitrary code execution through MCP."))
           << Qt::endl;
}

} // namespace

int icpp::runApplication(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("icpp"));
    QCoreApplication::setApplicationVersion(QString::fromLatin1(ICPP_VERSION));
    const icpp::HelpLanguage commandLineHelpLanguage = initialHelpLanguage(argc, argv);

    QCommandLineParser commandLineParser;
    commandLineParser.setApplicationDescription(trMessage(
        commandLineHelpLanguage,
        QStringLiteral("cling / qtcling を使いやすくする対話型 C++ REPL ラッパー。"),
        QStringLiteral("Interactive C++ REPL wrapper for cling and qtcling.")));

    const QCommandLineOption helpOption(QStringList() << QStringLiteral("h") << QStringLiteral("help"),
                                        trMessage(commandLineHelpLanguage,
                                                  QStringLiteral("ヘルプを表示します。"),
                                                  QStringLiteral("Show this help.")));
    const QCommandLineOption versionOption(QStringList() << QStringLiteral("v") << QStringLiteral("version"),
                                           trMessage(commandLineHelpLanguage,
                                                     QStringLiteral("バージョンを表示します。"),
                                                     QStringLiteral("Show version information.")));
    const QCommandLineOption engineOption(QStringList() << QStringLiteral("engine"),
                                          trMessage(commandLineHelpLanguage,
                                                    QStringLiteral("interpreter engine を選択します: cling または qtcling。"),
                                                    QStringLiteral("Select the interpreter engine: cling or qtcling.")),
                                          QStringLiteral("engine"),
                                          QStringLiteral("qtcling"));
    const QCommandLineOption clingOption(QStringList() << QStringLiteral("cling"),
                                         trMessage(commandLineHelpLanguage,
                                                   QStringLiteral("cling 実行ファイルのパス。"),
                                                   QStringLiteral("Path to the cling executable.")),
                                         QStringLiteral("path"));
    const QCommandLineOption qtClingOption(QStringList() << QStringLiteral("qtcling"),
                                           trMessage(commandLineHelpLanguage,
                                                     QStringLiteral("qtcling 実行ファイルのパス。"),
                                                     QStringLiteral("Path to the qtcling executable.")),
                                           QStringLiteral("path"));
    const QCommandLineOption interpreterArgumentOption(
        QStringList() << QStringLiteral("interpreter-arg"),
        trMessage(commandLineHelpLanguage,
                  QStringLiteral("選択中の interpreter に渡す追加引数。"),
                  QStringLiteral("Additional argument passed to the selected interpreter.")),
        QStringLiteral("arg"));
    const QCommandLineOption clingArgumentOption(
        QStringList() << QStringLiteral("cling-arg"),
        trMessage(commandLineHelpLanguage,
                  QStringLiteral("--engine cling のときだけ cling に渡す追加引数。"),
                  QStringLiteral("Additional argument passed to cling when --engine cling is selected.")),
        QStringLiteral("arg"));
    const QCommandLineOption qtClingArgumentOption(
        QStringList() << QStringLiteral("qtcling-arg"),
        trMessage(commandLineHelpLanguage,
                  QStringLiteral("--engine qtcling のときだけ qtcling に渡す追加引数。"),
                  QStringLiteral("Additional argument passed to qtcling when --engine qtcling is selected.")),
        QStringLiteral("arg"));
    const QCommandLineOption helpLanguageOption(
        QStringList() << QStringLiteral("help-language"),
        trMessage(commandLineHelpLanguage,
                  QStringLiteral("icpp のヘルプと警告の言語: ja または en。"),
                  QStringLiteral("Language for icpp help and warnings: ja or en.")),
        QStringLiteral("language"));
    const QCommandLineOption mcpHttpOption(
        QStringList() << QStringLiteral("mcp-http"),
        trMessage(commandLineHelpLanguage,
                  QStringLiteral("ローカル Streamable HTTP MCP Server として起動します。"),
                  QStringLiteral("Run as a local Streamable HTTP MCP server.")));
    const QCommandLineOption mcpPortOption(
        QStringList() << QStringLiteral("mcp-port"),
        trMessage(commandLineHelpLanguage,
                  QStringLiteral("MCP HTTP port。0 を指定すると空き port を選択します。"),
                  QStringLiteral("MCP HTTP port. Use 0 to select an available port.")),
        QStringLiteral("port"),
        QString::number(icpp::McpHttpServer::DefaultPort));
    const QCommandLineOption mcpAllowExecutionOption(
        QStringList() << QStringLiteral("mcp-allow-execution"),
        trMessage(commandLineHelpLanguage,
                  QStringLiteral("MCP からの任意コード実行を明示的に許可します。"),
                  QStringLiteral("Explicitly allow arbitrary code execution through MCP.")));

    commandLineParser.addOption(helpOption);
    commandLineParser.addOption(versionOption);
    commandLineParser.addOption(engineOption);
    commandLineParser.addOption(clingOption);
    commandLineParser.addOption(qtClingOption);
    commandLineParser.addOption(interpreterArgumentOption);
    commandLineParser.addOption(clingArgumentOption);
    commandLineParser.addOption(qtClingArgumentOption);
    commandLineParser.addOption(helpLanguageOption);
    commandLineParser.addOption(mcpHttpOption);
    commandLineParser.addOption(mcpPortOption);
    commandLineParser.addOption(mcpAllowExecutionOption);
    if (!commandLineParser.parse(application.arguments())) {
        standardError() << trMessage(commandLineHelpLanguage,
                                     QStringLiteral("コマンドラインオプションが不正です。--help を確認してください。"),
                                     QStringLiteral("Invalid command line options. Use --help."))
                        << Qt::endl;
        return 1;
    }

    if (commandLineParser.isSet(helpOption)) {
        showCommandLineHelp(commandLineHelpLanguage);
        return 0;
    }
    if (commandLineParser.isSet(versionOption)) {
        standardOutput() << QStringLiteral("icpp %1").arg(QCoreApplication::applicationVersion())
                         << Qt::endl;
        return 0;
    }

    const std::optional<icpp::Engine> parsedEngine =
        icpp::parseEngineName(commandLineParser.value(engineOption));
    if (!parsedEngine.has_value()) {
        standardError() << trMessage(commandLineHelpLanguage,
                                     QStringLiteral("--engine の値が不正です。'cling' または 'qtcling' を指定してください。"),
                                     QStringLiteral("Invalid value for --engine. Use 'cling' or 'qtcling'."))
                        << Qt::endl;
        return 1;
    }

    QString helpLanguageName = commandLineParser.value(helpLanguageOption);
    if (helpLanguageName.isEmpty()) {
        helpLanguageName = qEnvironmentVariable("ICPP_HELP_LANGUAGE");
    }
    if (helpLanguageName.isEmpty()) {
        helpLanguageName = storedHelpLanguageName();
    }
    const std::optional<icpp::HelpLanguage> parsedHelpLanguage =
        icpp::parseHelpLanguageName(helpLanguageName);
    if (!parsedHelpLanguage.has_value()) {
        standardError() << trMessage(commandLineHelpLanguage,
                                     QStringLiteral("help language の値が不正です。'ja' または 'en' を指定してください。"),
                                     QStringLiteral("Invalid value for help language. Use 'ja' or 'en'."))
                        << Qt::endl;
        return 1;
    }

    const QString explicitProgram = *parsedEngine == icpp::Engine::Cling
        ? commandLineParser.value(clingOption)
        : commandLineParser.value(qtClingOption);
    const QString interpreterProgram =
        icpp::resolveInterpreterProgram(*parsedEngine,
                                        explicitProgram,
                                        *parsedHelpLanguage,
                                        standardError());
    if (interpreterProgram.isEmpty()) {
        return 1;
    }

    QStringList interpreterArguments =
        parseRepeatedOptionValues(commandLineParser, interpreterArgumentOption);
    interpreterArguments.append(parseRepeatedOptionValues(
        commandLineParser,
        *parsedEngine == icpp::Engine::Cling ? clingArgumentOption : qtClingArgumentOption));

    if (commandLineParser.isSet(mcpAllowExecutionOption)
        && !commandLineParser.isSet(mcpHttpOption)) {
        standardError() << trMessage(
                               commandLineHelpLanguage,
                               QStringLiteral("--mcp-allow-execution は --mcp-http と一緒に指定してください。"),
                               QStringLiteral("--mcp-allow-execution requires --mcp-http."))
                        << Qt::endl;
        return 1;
    }

    if (commandLineParser.isSet(mcpHttpOption)) {
        bool validPort = false;
        const uint requestedPort = commandLineParser.value(mcpPortOption).toUInt(&validPort);
        if (!validPort || requestedPort > 65535) {
            standardError() << trMessage(commandLineHelpLanguage,
                                         QStringLiteral("--mcp-port には 0 から 65535 を指定してください。"),
                                         QStringLiteral("--mcp-port must be between 0 and 65535."))
                            << Qt::endl;
            return 1;
        }

        const QByteArray authenticationToken = qgetenv("ICPP_MCP_TOKEN");
        if (authenticationToken.size() < 32 || authenticationToken.size() > 4096) {
            standardError() << trMessage(
                                   commandLineHelpLanguage,
                                   QStringLiteral("ICPP_MCP_TOKEN に 32 文字以上 4096 文字以下の token を設定してください。"),
                                   QStringLiteral("Set ICPP_MCP_TOKEN to a token between 32 and 4096 bytes."))
                            << Qt::endl;
            return 1;
        }

        const bool executionAllowed = commandLineParser.isSet(mcpAllowExecutionOption);
        icpp::McpHttpServer mcpServer(*parsedEngine,
                                      interpreterProgram,
                                      interpreterArguments,
                                      authenticationToken,
                                      executionAllowed);
        if (!mcpServer.start(quint16(requestedPort))) {
            standardError() << trMessage(commandLineHelpLanguage,
                                         QStringLiteral("MCP HTTP Server を起動できません: %1"),
                                         QStringLiteral("Could not start MCP HTTP server: %1"))
                                   .arg(mcpServer.errorString())
                            << Qt::endl;
            return 1;
        }

        standardError() << trMessage(commandLineHelpLanguage,
                                     QStringLiteral("icpp MCP endpoint: %1"),
                                     QStringLiteral("icpp MCP endpoint: %1"))
                               .arg(mcpServer.endpointUrl())
                        << Qt::endl;
        if (executionAllowed) {
            standardError() << trMessage(
                                   commandLineHelpLanguage,
                                   QStringLiteral("警告: MCP クライアントに利用者権限での任意コード実行を許可しています。"),
                                   QStringLiteral("Warning: MCP clients may execute arbitrary code with your user permissions."))
                            << Qt::endl;
        } else {
            standardError() << trMessage(
                                   commandLineHelpLanguage,
                                   QStringLiteral("MCP コード実行は無効です。status tool だけを使用できます。"),
                                   QStringLiteral("MCP code execution is disabled; only the status tool can run."))
                            << Qt::endl;
        }
        return application.exec();
    }

    icpp::ReplSession replSession(*parsedEngine,
                                  interpreterProgram,
                                  interpreterArguments,
                                  *parsedHelpLanguage,
                                  standardOutput(),
                                  standardError());
    return replSession.run();
}
