#include "BoardPersistence.h"
#include "BoardDocumentStorage.h"
#include "BoardModel.h"

#include <QDir>
#include <QFileInfo>
#include <QObject>

BoardPersistence::BoardPersistence(const QString &path, Reader reader, Writer writer)
    : m_path(path), m_reader(reader ? std::move(reader) : BoardDocumentStorage::read),
      m_writer(writer ? std::move(writer) : BoardDocumentStorage::write)
{
}

bool BoardPersistence::restore(BoardModel &model, bool retry)
{
    // Reload is only permitted before a usable document exists; never discard edits.
    if ((retry && m_state != State::RestoreBlocked) || (!retry && m_state != State::Restoring))
        return false;
    m_state = State::Restoring;
    BoardDocument candidate;
    m_result = m_reader(m_path, &candidate);
    if (m_result.ok()) {
        model = BoardModel();
        model.setDocument(candidate);
        m_savedSnapshot = model.document().toJson();
        m_hasSavedSnapshot = true;
        m_state = State::Ready;
        return true;
    }
    if (m_result.missing() && !retry) {
        model = BoardModel();
        m_hasSavedSnapshot = false;
        if (QDir().mkpath(QFileInfo(m_path).absolutePath())) {
            m_state = State::Ready;
            return true;
        }
        m_result.status = BoardFileResult::Status::Failure;
        m_result.stage = QStringLiteral("create_directory");
        m_result.message = QObject::tr("The save directory could not be created.");
        m_state = State::SaveFailed;
        return false;
    }
    if (m_result.missing()) {
        m_result.status = BoardFileResult::Status::Failure;
        m_result.stage = QStringLiteral("read");
        m_result.message = QObject::tr("The saved file is now missing. Restore it and reload, or preserve the original separately before restarting with a new document.");
    }
    m_state = State::RestoreBlocked;
    return false;
}

bool BoardPersistence::save(const BoardDocument &document, bool retry)
{
    if (!documentAvailable() || (m_state == State::SaveFailed && !retry))
        return false;
    m_result = m_writer(m_path, document);
    if (!m_result.ok()) {
        m_state = State::SaveFailed;
        return false;
    }
    m_savedSnapshot = document.toJson();
    m_hasSavedSnapshot = true;
    m_state = State::Ready;
    return true;
}

BoardPersistence::State BoardPersistence::state() const { return m_state; }
bool BoardPersistence::documentAvailable() const { return m_state == State::Ready || m_state == State::SaveFailed; }
QString BoardPersistence::filePath() const { return m_path; }
const BoardFileResult &BoardPersistence::lastResult() const { return m_result; }

QJsonObject BoardPersistence::status(const BoardDocument &document, bool pendingInput) const
{
    QString name;
    switch (m_state) {
    case State::Restoring: name = QStringLiteral("restoring"); break;
    case State::Ready: name = QStringLiteral("ready"); break;
    case State::SaveFailed: name = QStringLiteral("save_failed"); break;
    case State::RestoreBlocked: name = QStringLiteral("restore_blocked"); break;
    }
    QJsonObject result{{QStringLiteral("status"), name},
        {QStringLiteral("persisted"), m_state == State::Ready && m_hasSavedSnapshot
            && !pendingInput && document.toJson() == m_savedSnapshot}};
    if (m_state == State::SaveFailed || m_state == State::RestoreBlocked)
        result.insert(QStringLiteral("error"), m_result.errorJson());
    return result;
}

bool BoardPersistence::allowsOutput(const QString &path, QString *errorMessage) const
{
    if (BoardDocumentStorage::sameFile(path, m_path)) {
        if (errorMessage)
            *errorMessage = QObject::tr("Choose a different file. The application's saved document cannot be replaced by an export.");
        return false;
    }
    return true;
}
