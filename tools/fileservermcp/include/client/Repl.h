#pragma once

class CommandRegistry;
class QTextStream;

class Repl
{
public:
    Repl(CommandRegistry *registry, QTextStream &out, QTextStream &err);

    int run();

private:
    CommandRegistry *m_registry = nullptr;
    QTextStream &m_out;
    QTextStream &m_err;
};
