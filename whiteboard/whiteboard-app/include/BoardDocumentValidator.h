#pragma once

#include <QJsonObject>
#include <QString>

class BoardDocument;

class BoardDocumentValidator {
public:
    static bool validate(const QJsonObject &object, QString *error);
    static bool validateConnections(const BoardDocument &document, QString *error);
};
