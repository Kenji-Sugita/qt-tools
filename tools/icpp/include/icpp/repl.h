#pragma once

#include <QString>
#include <QStringList>

#include <memory>
#include <optional>

class QTextStream;

namespace icpp {

enum class Engine {
    Cling,
    QtCling,
};

enum class HelpLanguage {
    Japanese,
    English,
};

QString engineName(Engine engine);

std::optional<Engine> parseEngineName(const QString& value);

std::optional<HelpLanguage> parseHelpLanguageName(const QString& value);

QString resolveInterpreterProgram(Engine engine,
                                  const QString& commandLinePath,
                                  HelpLanguage helpLanguage,
                                  QTextStream& errorStream);

class ReplSession
{
public:
    ReplSession(Engine engine,
                QString programPath,
                QStringList interpreterArguments,
                HelpLanguage helpLanguage,
                QTextStream& output,
                QTextStream& error);
    ~ReplSession();

    ReplSession(const ReplSession&) = delete;
    ReplSession& operator=(const ReplSession&) = delete;
    ReplSession(ReplSession&&) noexcept;
    ReplSession& operator=(ReplSession&&) noexcept;

    int run();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace icpp
