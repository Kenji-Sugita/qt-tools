#include "icpp/repl.h"

#include "icpp/ReplSessionImpl.h"

#include <QTextStream>

#include <memory>
#include <optional>
#include <utility>

namespace icpp {

class ReplSession::Impl : public ReplSessionImpl
{
public:
    using ReplSessionImpl::ReplSessionImpl;
};

QString engineName(Engine engine)
{
    return engine == Engine::Cling ? QStringLiteral("cling") : QStringLiteral("qtcling");
}

std::optional<Engine> parseEngineName(const QString& value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("cling")) {
        return Engine::Cling;
    }
    if (normalized.isEmpty() || normalized == QStringLiteral("qtcling")) {
        return Engine::QtCling;
    }
    return std::nullopt;
}

std::optional<HelpLanguage> parseHelpLanguageName(const QString& value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized.isEmpty() || normalized == QStringLiteral("ja")
        || normalized == QStringLiteral("jp") || normalized == QStringLiteral("japanese")) {
        return HelpLanguage::Japanese;
    }
    if (normalized == QStringLiteral("en") || normalized == QStringLiteral("english")) {
        return HelpLanguage::English;
    }
    return std::nullopt;
}

QString resolveInterpreterProgram(Engine engine,
                                  const QString& commandLinePath,
                                  HelpLanguage helpLanguage,
                                  QTextStream& errorStream)
{
    return resolveInterpreterProgramImpl(engine, commandLinePath, helpLanguage, errorStream);
}

ReplSession::ReplSession(Engine engine,
                         QString programPath,
                         QStringList interpreterArguments,
                         HelpLanguage helpLanguage,
                         QTextStream& output,
                         QTextStream& error)
    : impl(std::make_unique<Impl>(engine,
                                  std::move(programPath),
                                  std::move(interpreterArguments),
                                  helpLanguage,
                                  output,
                                  error))
{
}

ReplSession::~ReplSession() = default;

ReplSession::ReplSession(ReplSession&&) noexcept = default;

ReplSession& ReplSession::operator=(ReplSession&&) noexcept = default;

int ReplSession::run()
{
    return impl->run();
}

} // namespace icpp
