#pragma once

#include "BoardDocument.h"
#include "BoardFileResult.h"

#include <functional>

class QIODevice;

class BoardDocumentStorage {
public:
    static BoardFileResult read(const QString &filePath, BoardDocument *document);
    static BoardFileResult write(const QString &filePath, const BoardDocument &document);
    static bool sameFile(const QString &left, const QString &right);

    // The caller owns the open device and discards uncommitted output on failure.
    // These entry points share the exact validation and completion checks used by file I/O.
    static BoardFileResult readDevice(QIODevice &device, BoardDocument *document);
    static BoardFileResult writeDevice(QIODevice &device, const BoardDocument &document,
                                      const std::function<bool()> &commit);
};
