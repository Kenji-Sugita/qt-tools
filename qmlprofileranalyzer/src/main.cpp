#include "analyzer.h"
#include "qtdreader.h"
#include "report.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>

using namespace QmlProfilerAnalyzer;

namespace {

struct ParsedArgs
{
    QString command;
    QString filePath;
    QString secondFilePath;
};

ParsedArgs parseArgs(const QStringList &positionalArgs)
{
    ParsedArgs args;
    if (positionalArgs.size() == 1) {
        args.command = QStringLiteral("summary");
        args.filePath = positionalArgs.at(0);
    } else if (positionalArgs.size() == 2) {
        args.command = positionalArgs.at(0);
        args.filePath = positionalArgs.at(1);
    } else if (positionalArgs.size() == 3) {
        args.command = positionalArgs.at(0);
        args.filePath = positionalArgs.at(1);
        args.secondFilePath = positionalArgs.at(2);
    }
    return args;
}

bool isKnownCommand(const QString &command)
{
    return command == QLatin1String("summary")
        || command == QLatin1String("ranges")
        || command == QLatin1String("hotspots")
        || command == QLatin1String("types")
        || command == QLatin1String("events")
        || command == QLatin1String("events-aggregated")
        || command == QLatin1String("compare")
        || command == QLatin1String("convert")
        || command == QLatin1String("notes");
}

QString helpText(bool includeQtOptions)
{
    QString text;
    QTextStream stream(&text);
    stream << "Usage:\n";
    stream << "  qmlprofileranalyzer <trace.qtd|trace.qzt>\n";
    stream << "  qmlprofileranalyzer <command> <trace.qtd|trace.qzt>\n";
    stream << "  qmlprofileranalyzer compare <before.qtd|before.qzt> <after.qtd|after.qzt>\n";
    stream << "  qmlprofileranalyzer convert <input.qtd|input.qzt> <output.qzt|output.qtd>\n";
    stream << '\n';
    stream << "Read QML profiler .qtd/XML and .qzt/binary traces.\n";
    stream << '\n';
    stream << "Report commands:\n";
    stream << "  summary              Print a high-level trace summary.\n";
    stream << "  ranges               Print range events.\n";
    stream << "  hotspots             Print aggregated costly ranges.\n";
    stream << "  types                Print event type definitions.\n";
    stream << "  events               Print events in timestamp order.\n";
    stream << "  events-aggregated    Print aggregated event statistics.\n";
    stream << "  notes                Print profiler notes.\n";
    stream << '\n';
    stream << "Other commands:\n";
    stream << "  compare              Compare two traces by hotspot group.\n";
    stream << "  convert              Convert .qzt to .qtd or .qtd to .qzt.\n";
    stream << '\n';
    stream << "Examples:\n";
    stream << "  qmlprofileranalyzer summary trace.qzt\n";
    stream << "  qmlprofileranalyzer hotspots trace.qtd\n";
    stream << "  qmlprofileranalyzer compare before.qzt after.qzt\n";
    stream << "  qmlprofileranalyzer convert trace.qzt trace.qtd\n";
    stream << "  qmlprofileranalyzer convert trace.qtd trace.qzt\n";
    stream << '\n';
    stream << "Options:\n";
    stream << "  -h, --help           Displays this help.\n";
    stream << "  --help-all           Displays this help, including generic Qt options.\n";
    stream << "  -v, --version        Displays version information.\n";
    if (includeQtOptions) {
        stream << "  --qmljsdebugger <value>\n";
        stream << "                       Activates the QML/JS debugger with a specified port.\n";
        stream << "                       The value must be of format port:1234[,block].\n";
        stream << "                       \"block\" makes the application wait for a connection.\n";
    }
    return text;
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("qmlprofileranalyzer"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Read QML profiler .qtd/XML and .qzt/binary traces and print a text report."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("command"),
                                 QStringLiteral("summary | ranges | hotspots | types | events | events-aggregated | compare | convert | notes"));
    parser.addPositionalArgument(QStringLiteral("file"),
                                 QStringLiteral("Path to a .qtd/XML or .qzt/binary trace file."));
    parser.addPositionalArgument(QStringLiteral("output"),
                                 QStringLiteral("Output trace path for convert."),
                                 QStringLiteral("[output]"));

    const QStringList rawArgs = app.arguments();
    if (rawArgs.contains(QStringLiteral("-h")) || rawArgs.contains(QStringLiteral("--help"))) {
        QTextStream(stdout) << helpText(false);
        return 0;
    }
    if (rawArgs.contains(QStringLiteral("--help-all"))) {
        QTextStream(stdout) << helpText(true);
        return 0;
    }

    parser.process(app);

    const ParsedArgs args = parseArgs(parser.positionalArguments());
    QTextStream out(stdout);
    QTextStream err(stderr);

    if (args.filePath.isEmpty() || !isKnownCommand(args.command)
        || (!args.secondFilePath.isEmpty() && args.command != QLatin1String("compare")
            && args.command != QLatin1String("convert"))) {
        err << helpText(false);
        return 1;
    }

    const QFileInfo fileInfo(args.filePath);
    if (!fileInfo.exists()) {
        err << "Input file does not exist: " << args.filePath << '\n';
        return 1;
    }

    QtdReader reader;
    const auto trace = reader.readFile(args.filePath);
    if (!trace.has_value()) {
        err << reader.errorString() << '\n';
        return 1;
    }

    if (args.command == QLatin1String("convert")) {
        if (args.secondFilePath.isEmpty()) {
            err << "Convert command requires an input file and an output file.\n";
            return 1;
        }

        const QString inputSuffix = fileInfo.suffix().toLower();
        const QString outputSuffix = QFileInfo(args.secondFilePath).suffix().toLower();
        const bool convertQztToQtd =
            inputSuffix == QLatin1String("qzt") && outputSuffix == QLatin1String("qtd");
        const bool convertQtdToQzt =
            inputSuffix == QLatin1String("qtd") && outputSuffix == QLatin1String("qzt");
        if (!convertQztToQtd && !convertQtdToQzt) {
            err << "Convert supports <input.qzt> <output.qtd> and <input.qtd> <output.qzt>.\n";
            return 1;
        }

        const QFileInfo outputFileInfo(args.secondFilePath);
        if (!outputFileInfo.dir().exists()) {
            err << "Output directory does not exist: "
                << outputFileInfo.dir().absolutePath() << '\n';
            return 1;
        }

        const bool written = convertQztToQtd
            ? reader.writeQtdFile(trace.value(), args.secondFilePath)
            : reader.writeQztFile(trace.value(), args.secondFilePath);
        if (!written) {
            err << reader.errorString() << '\n';
            return 1;
        }

        out << "Converted " << args.filePath << " to " << args.secondFilePath << '\n';
        return 0;
    }

    if (args.command == QLatin1String("compare")) {
        if (args.secondFilePath.isEmpty()) {
            err << "Compare command requires two input files.\n";
            return 1;
        }

        const QFileInfo secondFileInfo(args.secondFilePath);
        if (!secondFileInfo.exists()) {
            err << "Input file does not exist: " << args.secondFilePath << '\n';
            return 1;
        }

        const auto secondTrace = reader.readFile(args.secondFilePath);
        if (!secondTrace.has_value()) {
            err << reader.errorString() << '\n';
            return 1;
        }

        out << Report::renderHotspotComparison(
                   trace.value(),
                   secondTrace.value(),
                   Analyzer::compareHotspots(trace.value(), secondTrace.value()))
            << '\n';
        return 0;
    }

    const Summary summary = Analyzer::summarize(trace.value());
    const RangeAnalysis rangeAnalysis = Analyzer::analyzeRanges(trace.value());
    const HotspotAnalysis hotspotAnalysis = Analyzer::analyzeHotspots(trace.value());

    if (args.command == QLatin1String("summary"))
        out << Report::renderSummary(trace.value(), summary) << '\n';
    else if (args.command == QLatin1String("ranges"))
        out << Report::renderRanges(trace.value(), rangeAnalysis) << '\n';
    else if (args.command == QLatin1String("hotspots"))
        out << Report::renderHotspots(trace.value(), hotspotAnalysis) << '\n';
    else if (args.command == QLatin1String("types"))
        out << Report::renderTypes(trace.value(), summary) << '\n';
    else if (args.command == QLatin1String("events"))
        out << Report::renderEvents(trace.value()) << '\n';
    else if (args.command == QLatin1String("events-aggregated"))
        out << Report::renderAggregatedEvents(trace.value()) << '\n';
    else if (args.command == QLatin1String("notes"))
        out << Report::renderNotes(trace.value()) << '\n';

    return 0;
}
