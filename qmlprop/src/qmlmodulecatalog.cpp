#include "qmlmodulecatalog.h"

#include <QStringList>

namespace {

ImportProfile makeProfile(const QString& description, const QStringList& importStatements)
{
    ImportProfile importProfile;
    importProfile.description = description;
    importProfile.importStatements = importStatements;
    return importProfile;
}

QStringList appendImport(const QStringList& importStatements, const QString& importStatement)
{
    QStringList combinedImportStatements = importStatements;
    combinedImportStatements.append(importStatement);
    return combinedImportStatements;
}

QStringList appendImports(const QStringList& importStatements, const QStringList& additionalImportStatements)
{
    QStringList combinedImportStatements = importStatements;
    combinedImportStatements.append(additionalImportStatements);
    return combinedImportStatements;
}

QStringList importedModuleUris(const ImportProfile& importProfile)
{
    QStringList moduleUris;
    for (const QString& importStatement : importProfile.importStatements) {
        if (!importStatement.startsWith(QStringLiteral("import "))) {
            continue;
        }

        QString remainder = importStatement.mid(QStringLiteral("import ").size()).trimmed();
        const qsizetype asIndex = remainder.indexOf(QStringLiteral(" as "));
        if (asIndex >= 0) {
            remainder = remainder.left(asIndex).trimmed();
        }

        const QString importedModuleUri = remainder.section(QLatin1Char(' '), 0, 0).trimmed();
        if (!importedModuleUri.isEmpty() && !moduleUris.contains(importedModuleUri)) {
            moduleUris.append(importedModuleUri);
        }
    }
    return moduleUris;
}

} // namespace

QStringList QmlModuleCatalog::baseImports()
{
    return {
        QStringLiteral("import QtCore"),
        QStringLiteral("import QtQml"),
        QStringLiteral("import QtQuick")
    };
}

QString QmlModuleCatalog::qualifierFromTypeName(const QString& requestedTypeName)
{
    const qsizetype dotIndex = requestedTypeName.indexOf(u'.');
    if (dotIndex <= 0) {
        return QString();
    }

    return requestedTypeName.left(dotIndex);
}

QList<ImportProfile> QmlModuleCatalog::profilesForType(const QString& requestedTypeName)
{
    QList<ImportProfile> importProfiles;
    const QStringList commonImports = baseImports();
    const QString qualifier = qualifierFromTypeName(requestedTypeName);

    if (qualifier == QStringLiteral("T")) {
        importProfiles.append(makeProfile(QStringLiteral("QtQuick.Templates alias"), {
            QStringLiteral("import QtCore"),
            QStringLiteral("import QtQml"),
            QStringLiteral("import QtQuick"),
            QStringLiteral("import QtQuick.Templates as T")
        }));
    }

    if (qualifier == QStringLiteral("QLabP")) {
        importProfiles.append(makeProfile(QStringLiteral("Qt.labs.platform alias"), {
            QStringLiteral("import QtCore"),
            QStringLiteral("import QtQml"),
            QStringLiteral("import QtQuick"),
            QStringLiteral("import Qt.labs.platform as QLabP")
        }));
    }

    importProfiles.append(makeProfile(QStringLiteral("Base modules"), commonImports));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Controls"), appendImport(commonImports, QStringLiteral("import QtQuick.Controls"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Templates"), appendImport(commonImports, QStringLiteral("import QtQuick.Templates"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Templates alias"), appendImport(commonImports, QStringLiteral("import QtQuick.Templates as T"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Dialogs"), appendImport(commonImports, QStringLiteral("import QtQuick.Dialogs"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Pdf"), appendImport(commonImports, QStringLiteral("import QtQuick.Pdf"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Shapes"), appendImport(commonImports, QStringLiteral("import QtQuick.Shapes"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Layouts"), appendImport(commonImports, QStringLiteral("import QtQuick.Layouts"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Window"), appendImport(commonImports, QStringLiteral("import QtQuick.Window"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Particles"), appendImport(commonImports, QStringLiteral("import QtQuick.Particles"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.LocalStorage"), appendImport(commonImports, QStringLiteral("import QtQuick.LocalStorage"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Scene2D"), appendImport(commonImports, QStringLiteral("import QtQuick.Scene2D"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Scene3D"), appendImport(commonImports, QStringLiteral("import QtQuick.Scene3D"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.VirtualKeyboard"), appendImport(commonImports, QStringLiteral("import QtQuick.VirtualKeyboard"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Timeline"), appendImport(commonImports, QStringLiteral("import QtQuick.Timeline"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick.Effects"), appendImport(commonImports, QStringLiteral("import QtQuick.Effects"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick3D"), appendImport(commonImports, QStringLiteral("import QtQuick3D"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick3D.Effects"), appendImports(commonImports, QStringList{
        QStringLiteral("import QtQuick3D"),
        QStringLiteral("import QtQuick3D.Effects")
    })));
    importProfiles.append(makeProfile(QStringLiteral("QtQuick3D.Helpers"), appendImports(commonImports, QStringList{
        QStringLiteral("import QtQuick3D"),
        QStringLiteral("import QtQuick3D.Helpers")
    })));
    importProfiles.append(makeProfile(QStringLiteral("Qt3D"), appendImports(commonImports, QStringList{
        QStringLiteral("import Qt3D.Core"),
        QStringLiteral("import Qt3D.Input"),
        QStringLiteral("import Qt3D.Logic"),
        QStringLiteral("import Qt3D.Render"),
        QStringLiteral("import Qt3D.Extras")
    })));
    importProfiles.append(makeProfile(QStringLiteral("QtCharts"), appendImport(commonImports, QStringLiteral("import QtCharts"))));
    importProfiles.append(makeProfile(QStringLiteral("QtDataVisualization"), appendImport(commonImports, QStringLiteral("import QtDataVisualization"))));
    importProfiles.append(makeProfile(QStringLiteral("QtMultimedia"), appendImport(commonImports, QStringLiteral("import QtMultimedia"))));
    importProfiles.append(makeProfile(QStringLiteral("QtPositioning"), appendImport(commonImports, QStringLiteral("import QtPositioning"))));
    importProfiles.append(makeProfile(QStringLiteral("QtLocation"), appendImports(commonImports, QStringList{
        QStringLiteral("import QtPositioning"),
        QStringLiteral("import QtLocation")
    })));
    importProfiles.append(makeProfile(QStringLiteral("QtSensors"), appendImport(commonImports, QStringLiteral("import QtSensors"))));
    importProfiles.append(makeProfile(QStringLiteral("QtScxml"), appendImport(commonImports, QStringLiteral("import QtScxml"))));
    importProfiles.append(makeProfile(QStringLiteral("QtWebChannel"), appendImport(commonImports, QStringLiteral("import QtWebChannel"))));
    importProfiles.append(makeProfile(QStringLiteral("QtWebEngine"), appendImport(commonImports, QStringLiteral("import QtWebEngine"))));
    importProfiles.append(makeProfile(QStringLiteral("QtWebView"), appendImport(commonImports, QStringLiteral("import QtWebView"))));
    importProfiles.append(makeProfile(QStringLiteral("QtWebSockets"), appendImport(commonImports, QStringLiteral("import QtWebSockets"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQml.Models"), appendImport(commonImports, QStringLiteral("import QtQml.Models"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQml.WorkerScript"), appendImport(commonImports, QStringLiteral("import QtQml.WorkerScript"))));
    importProfiles.append(makeProfile(QStringLiteral("QtQml.StateMachine"), appendImport(commonImports, QStringLiteral("import QtQml.StateMachine"))));
    importProfiles.append(makeProfile(QStringLiteral("Qt5Compat.GraphicalEffects"), appendImport(commonImports, QStringLiteral("import Qt5Compat.GraphicalEffects"))));
    importProfiles.append(makeProfile(QStringLiteral("Qt.labs.folderlistmodel"), appendImport(commonImports, QStringLiteral("import Qt.labs.folderlistmodel"))));
    importProfiles.append(makeProfile(QStringLiteral("Qt.labs.settings"), appendImport(commonImports, QStringLiteral("import Qt.labs.settings"))));
    importProfiles.append(makeProfile(QStringLiteral("Qt.labs.platform"), appendImport(commonImports, QStringLiteral("import Qt.labs.platform"))));
    importProfiles.append(makeProfile(QStringLiteral("Qt.labs.platform alias"), appendImport(commonImports, QStringLiteral("import Qt.labs.platform as QLabP"))));
    importProfiles.append(makeProfile(QStringLiteral("Qt.labs.qmlmodels"), appendImport(commonImports, QStringLiteral("import Qt.labs.qmlmodels"))));
    importProfiles.append(makeProfile(QStringLiteral("Qt.labs.animation"), appendImport(commonImports, QStringLiteral("import Qt.labs.animation"))));
    importProfiles.append(makeProfile(QStringLiteral("Qt.labs.wavefrontmesh"), appendImport(commonImports, QStringLiteral("import Qt.labs.wavefrontmesh"))));
    importProfiles.append(makeProfile(QStringLiteral("QtTest"), appendImport(commonImports, QStringLiteral("import QtTest"))));

    return importProfiles;
}


QList<ImportProfile> QmlModuleCatalog::profilesForTypeInModule(const QString& requestedTypeName,
                                                               const QString& moduleUri)
{
    QList<ImportProfile> matchingProfiles;
    const QList<ImportProfile> importProfiles = profilesForType(requestedTypeName);
    for (const ImportProfile& importProfile : importProfiles) {
        if (importedModuleUris(importProfile).contains(moduleUri)) {
            matchingProfiles.append(importProfile);
        }
    }
    return matchingProfiles;
}
