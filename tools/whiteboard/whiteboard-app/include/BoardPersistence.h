#pragma once

#include "BoardDocument.h"
#include "BoardFileResult.h"

#include <functional>

class BoardModel;

class BoardPersistence {
public:
    enum class State { Restoring, Ready, SaveFailed, RestoreBlocked };
    using Reader = std::function<BoardFileResult(const QString &, BoardDocument *)>;
    using Writer = std::function<BoardFileResult(const QString &, const BoardDocument &)>;

    explicit BoardPersistence(const QString &path, Reader reader = {}, Writer writer = {});
    bool restore(BoardModel &model, bool retry = false);
    bool save(const BoardDocument &document, bool retry = false);
    State state() const;
    bool documentAvailable() const;
    QString filePath() const;
    const BoardFileResult &lastResult() const;
    QJsonObject status(const BoardDocument &document, bool pendingInput = false) const;
    bool allowsOutput(const QString &path, QString *errorMessage) const;

private:
    QString m_path;
    Reader m_reader;
    Writer m_writer;
    State m_state = State::Restoring;
    BoardFileResult m_result;
    QJsonObject m_savedSnapshot;
    bool m_hasSavedSnapshot = false;
};
