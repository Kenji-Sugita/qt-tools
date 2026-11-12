#pragma once

#include "BoardDocument.h"

// Portable drawing data, independent of this machine's editing preferences.
class BoardDocumentExchange {
public:
    enum class Scope { AllPages, CurrentPage };

    static bool read(const QString &filePath, QVector<Page> *pages, QString *errorMessage);
    static bool write(const QString &filePath, const BoardDocument &document,
                      Scope scope, QString *errorMessage);
    static bool decode(const QByteArray &data, QVector<Page> *pages, QString *errorMessage);
    static bool validatePages(const QVector<Page> &pages, QString *errorMessage);
};
