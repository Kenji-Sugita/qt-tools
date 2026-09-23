#include "icpp/ReplSessionImpl.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QLibraryInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>
#include <QXmlStreamReader>

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <utility>

namespace icpp {

void ReplSessionImpl::previewUiFile(const QString& commandName,
                   const QString& filePathArgument,
                   const QString& historyEntry)
{
    if (!requireQtClingCommand(commandName)) {
        return;
    }

    const QString filePath = filePathArgument.trimmed();
    if (filePath.isEmpty()) {
        reportError(trMessage(QStringLiteral("%1 には .ui ファイルパスが必要です。"),
                              QStringLiteral("%1 requires a .ui file path."))
                        .arg(commandName));
        return;
    }

    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        reportError(trMessage(QStringLiteral("%1: .ui ファイルが見つかりません。"),
                              QStringLiteral("%1: .ui file not found."))
                        .arg(filePath));
        return;
    }

    const QString absoluteFilePath = fileInfo.absoluteFilePath();
    const QString loadFailedMessage =
        trMessage(QStringLiteral(".ui をロードできません: %1"),
                  QStringLiteral("Cannot load .ui file: %1"));
    const QString openedMessage =
        trMessage(QStringLiteral(".ui プレビューを開きました: %1"),
                  QStringLiteral("Opened .ui preview: %1"));
    const QString uiToolsLibraryPath = qtUiToolsLibraryPath();

    QString sourceCode = QStringLiteral(R"(%UITOOLS_LOAD%
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QWidget>
#include <QtUiTools/QUiLoader>
([]() {
const QString filePath = QStringLiteral("%UI_FILE%");
QFile file(filePath);
if (!file.open(QIODevice::ReadOnly)) {
    qDebug().noquote() << QString("%LOAD_FAILED%").arg(filePath);
    return;
}

QUiLoader loader;
QWidget* widget = loader.load(&file);
if (!widget) {
    qDebug().noquote() << QString("%LOAD_FAILED%").arg(filePath);
    return;
}

if (widget->windowTitle().isEmpty()) {
    widget->setWindowTitle(QFileInfo(filePath).fileName());
}
widget->setAttribute(Qt::WA_DeleteOnClose);
widget->show();
qDebug().noquote() << QString("%OPENED%").arg(filePath);
})();)");
    sourceCode.replace(QStringLiteral("%UITOOLS_LOAD%"),
                       uiToolsLibraryPath.isEmpty()
                           ? QString()
                           : QStringLiteral("#pragma cling load(\"%1\")")
                                 .arg(quoteForCppString(uiToolsLibraryPath)));
    sourceCode.replace(QStringLiteral("%UI_FILE%"), quoteForCppString(absoluteFilePath));
    sourceCode.replace(QStringLiteral("%LOAD_FAILED%"), quoteForCppString(loadFailedMessage));
    sourceCode.replace(QStringLiteral("%OPENED%"), quoteForCppString(openedMessage));
    interpreter.sendSource(sourceCode);
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::inspectTopLevelWidgets(const QString& historyEntry)
{
    if (!requireQtClingCommand(QStringLiteral(".inspect"))) {
        return;
    }

    const QString propertyEditorLibrary = propertyEditorLibraryPath();
    const QString propertyEditorInclude = propertyEditorIncludePath();

    if (propertyEditorLibrary.isEmpty()) {
#if defined(Q_OS_MACOS)
        const QString examplePath = QStringLiteral("/path/to/property_editor.dylib");
#else
        const QString examplePath = QStringLiteral("/path/to/property_editor.so");
#endif
        reportError(trMessage(QStringLiteral("property_editor library が未設定です。ICPP_PROPERTY_EDITOR_LIB=%1 を指定してください。"),
                              QStringLiteral("property_editor library is not configured. Set ICPP_PROPERTY_EDITOR_LIB=%1."))
                        .arg(examplePath));
        return;
    }

    if (propertyEditorInclude.isEmpty()) {
        reportError(trMessage(QStringLiteral("propertyeditor include path が未設定です。ICPP_PROPERTY_EDITOR_INCLUDE=/path/to/include を指定してください。"),
                              QStringLiteral("propertyeditor include path is not configured. Set ICPP_PROPERTY_EDITOR_INCLUDE=/path/to/include.")));
        return;
    }

    if (!QFileInfo::exists(propertyEditorLibrary)) {
        reportError(trMessage(QStringLiteral("property_editor library が見つかりません: %1"),
                              QStringLiteral("property_editor library not found: %1"))
                        .arg(propertyEditorLibrary));
        return;
    }

    if (!QFileInfo(propertyEditorInclude).isDir()) {
        reportError(trMessage(QStringLiteral("propertyeditor include path が見つかりません: %1"),
                              QStringLiteral("propertyeditor include path not found: %1"))
                        .arg(propertyEditorInclude));
        return;
    }

    const QString openedMessage =
        trMessage(QStringLiteral("inspector を開きました。"),
                  QStringLiteral("Opened inspector."));
    const QString noTargetMessage =
        trMessage(QStringLiteral("調べる top-level QWidget がありません。"),
                  QStringLiteral("No top-level QWidget to inspect."));

    QString sourceCode = QStringLiteral(R"ICPP(#pragma cling add_include_path("%PROPERTY_EDITOR_INCLUDE%")
#pragma cling load("%PROPERTY_EDITOR_LIB%")
#include "objectpicker.h"
#include "objecttreedialog.h"
#include "objecttreewidget.h"
#include "propertyeditor.h"
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <memory>
#include <QPushButton>
#include <QPointer>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>
#include <vector>
([]() {
auto describeTopLevelWidget = [](QWidget* widget) {
    const QString title = widget->windowTitle().isEmpty()
        ? QStringLiteral("(no title)")
        : widget->windowTitle();
    return QString("0x%1 %2 \"%3\"")
        .arg(reinterpret_cast<quintptr>(widget), 0, 16)
        .arg(widget->metaObject()->className())
        .arg(title);
};

auto *inspectorWindow = new QWidget;
inspectorWindow->setProperty("_icppInspector", true);
inspectorWindow->setAttribute(Qt::WA_DeleteOnClose);
inspectorWindow->setWindowTitle(QStringLiteral("icpp Inspector"));
inspectorWindow->resize(780, 720);

auto *layout = new QVBoxLayout(inspectorWindow);
auto *toolbar = new QWidget(inspectorWindow);
auto *toolbarLayout = new QHBoxLayout(toolbar);
toolbarLayout->setContentsMargins(0, 0, 0, 0);

auto *widgetSelector = new QComboBox(toolbar);
auto *refreshButton = new QPushButton(QStringLiteral("Refresh Widgets"), toolbar);
auto *treeButton = new QPushButton(QStringLiteral("Show Object Tree"), toolbar);
auto *pickButton = new QPushButton(QStringLiteral("Pick Object"), toolbar);
auto *clearPickButton = new QPushButton(QStringLiteral("Clear Pick"), toolbar);
toolbarLayout->addWidget(widgetSelector, 1);
toolbarLayout->addWidget(refreshButton);
toolbarLayout->addWidget(treeButton);
toolbarLayout->addWidget(pickButton);
toolbarLayout->addWidget(clearPickButton);

auto *propertyEditor = new PropertyEditor(inspectorWindow);
auto *objectTreeDialog = new ObjectTreeDialog(inspectorWindow);
objectTreeDialog->setProperty("_icppInspector", true);
auto *objectPicker = new ObjectPicker(inspectorWindow);

layout->addWidget(toolbar);
layout->addWidget(propertyEditor, 1);

struct InspectorState {
    QWidget* inspectorWindow = nullptr;
    QComboBox* widgetSelector = nullptr;
    PropertyEditor* propertyEditor = nullptr;
    ObjectTreeDialog* objectTreeDialog = nullptr;
    ObjectPicker* objectPicker = nullptr;
    QPointer<QWidget> currentRootWidget;
    QMetaObject::Connection rootDestroyedConnection;

    QString describeTopLevelWidget(QWidget* widget) const
    {
        const QString title = widget->windowTitle().isEmpty()
            ? QStringLiteral("(no title)")
            : widget->windowTitle();
        return QString("0x%1 %2 \"%3\"")
            .arg(reinterpret_cast<quintptr>(widget), 0, 16)
            .arg(widget->metaObject()->className())
            .arg(title);
    }

    QWidget* currentSelectedTopLevelWidget() const
    {
        const quintptr pointerValue = widgetSelector->currentData().value<qulonglong>();
        QWidget* selectedWidget = reinterpret_cast<QWidget*>(pointerValue);
        if (selectedWidget == nullptr) {
            return nullptr;
        }

        const QWidgetList widgets = QApplication::topLevelWidgets();
        for (QWidget* widget : widgets) {
            if (widget == selectedWidget && widget->isVisible()
                && !widget->property("_icppInspector").toBool()) {
                return selectedWidget;
            }
        }

        return nullptr;
    }

    void setCurrentObject(QWidget* rootWidget)
    {
        if (rootDestroyedConnection) {
            QObject::disconnect(rootDestroyedConnection);
            rootDestroyedConnection = {};
        }

        currentRootWidget = rootWidget;
        propertyEditor->setInspectedObject(rootWidget);
        objectTreeDialog->setRootObject(rootWidget);
        objectTreeDialog->setCurrentObject(rootWidget);
        objectPicker->setSelectedObject(rootWidget);

        if (rootWidget != nullptr) {
            rootDestroyedConnection =
                QObject::connect(rootWidget, &QObject::destroyed, inspectorWindow,
                                 [this]() {
                                     objectPicker->stop();
                                     objectPicker->setSelectedObject(nullptr);
                                     propertyEditor->setInspectedObject(nullptr);
                                     objectTreeDialog->setRootObject(nullptr);
                                     objectTreeDialog->setCurrentObject(nullptr);
                                     currentRootWidget = nullptr;
                                 });
        }
    }

    void refreshWidgets()
    {
        QWidget* previousWidget = currentSelectedTopLevelWidget();
        widgetSelector->blockSignals(true);
        widgetSelector->clear();

        int selectedIndex = -1;
        const QWidgetList widgets = QApplication::topLevelWidgets();
        for (QWidget* widget : widgets) {
            if (widget == nullptr || !widget->isVisible()
                || widget->property("_icppInspector").toBool()) {
                continue;
            }
            const int itemIndex = widgetSelector->count();
            widgetSelector->addItem(describeTopLevelWidget(widget),
                                    QVariant::fromValue<qulonglong>(
                                        reinterpret_cast<quintptr>(widget)));
            if (widget == previousWidget) {
                selectedIndex = itemIndex;
            }
        }

        if (selectedIndex < 0 && widgetSelector->count() > 0) {
            selectedIndex = 0;
        }
        if (selectedIndex >= 0) {
            widgetSelector->setCurrentIndex(selectedIndex);
        }
        widgetSelector->blockSignals(false);

        setCurrentObject(currentSelectedTopLevelWidget());
    }
};

auto state = std::make_shared<InspectorState>();
state->inspectorWindow = inspectorWindow;
state->widgetSelector = widgetSelector;
state->propertyEditor = propertyEditor;
state->objectTreeDialog = objectTreeDialog;
state->objectPicker = objectPicker;
static std::vector<std::shared_ptr<InspectorState>> inspectorStates;
inspectorStates.push_back(state);

QObject::connect(inspectorWindow, &QObject::destroyed, inspectorWindow,
                 [state]() {
                     state->objectPicker->stop();
                     state->objectPicker->setSelectedObject(nullptr);
                 });

QObject::connect(widgetSelector, qOverload<int>(&QComboBox::currentIndexChanged),
                 inspectorWindow, [state](int) {
                     state->setCurrentObject(state->currentSelectedTopLevelWidget());
                 });
QObject::connect(refreshButton, &QPushButton::clicked, inspectorWindow,
                 [state]() {
                     state->refreshWidgets();
                 });
QObject::connect(treeButton, &QPushButton::clicked, inspectorWindow,
                 [state]() {
                     state->objectPicker->stop();
                     QWidget* rootWidget = state->currentRootWidget.data();
                     state->objectTreeDialog->setRootObject(rootWidget);
                     state->objectTreeDialog->setCurrentObject(
                         state->propertyEditor->inspectedObject());
                     state->objectTreeDialog->show();
                     state->objectTreeDialog->raise();
                     state->objectTreeDialog->activateWindow();
                 });
QObject::connect(pickButton, &QPushButton::clicked, inspectorWindow,
                 [state]() {
                     QWidget* rootWidget = state->currentRootWidget.data();
                     if (rootWidget == nullptr || !rootWidget->isVisible()) {
                         return;
                     }
                     if (state->objectTreeDialog->isVisible()) {
                         state->objectTreeDialog->hide();
                     }
                     state->objectPicker->start(rootWidget);
                 });
QObject::connect(clearPickButton, &QPushButton::clicked, inspectorWindow,
                 [state]() {
                     state->objectPicker->stop();
                     state->objectPicker->setSelectedObject(nullptr);
                 });
QObject::connect(objectTreeDialog->objectTreeWidget(), &ObjectTreeWidget::objectActivated,
                 inspectorWindow, [state](QObject* targetObject) {
                     state->propertyEditor->setInspectedObject(targetObject);
                     state->objectPicker->setSelectedObject(targetObject);
                     state->objectTreeDialog->setCurrentObject(targetObject);
                 });
QObject::connect(objectPicker, &ObjectPicker::objectPicked, inspectorWindow,
                 [state](QObject* targetObject) {
                     state->propertyEditor->setInspectedObject(targetObject);
                     state->objectTreeDialog->setCurrentObject(targetObject);
                 });

state->refreshWidgets();
inspectorWindow->show();
if (widgetSelector->count() == 0) {
    qDebug().noquote() << "%NO_TARGET%";
}
qDebug().noquote() << "%OPENED%";
})();)ICPP");
    sourceCode.replace(QStringLiteral("%PROPERTY_EDITOR_INCLUDE%"),
                       quoteForCppString(propertyEditorInclude));
    sourceCode.replace(QStringLiteral("%PROPERTY_EDITOR_LIB%"),
                       quoteForCppString(propertyEditorLibrary));
    sourceCode.replace(QStringLiteral("%NO_TARGET%"), quoteForCppString(noTargetMessage));
    sourceCode.replace(QStringLiteral("%OPENED%"), quoteForCppString(openedMessage));
    interpreter.sendSource(sourceCode);
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::showWidgets(const QString& argument, const QString& historyEntry)
{
    if (!requireQtClingCommand(QStringLiteral(".widgets"))) {
        return;
    }

    const QString normalizedArgument = argument.trimmed().toLower();
    if (!normalizedArgument.isEmpty() && normalizedArgument != QStringLiteral("all")) {
        reportError(trMessage(QStringLiteral(".widgets には 'all' を指定できます。"),
                              QStringLiteral(".widgets accepts only 'all'.")));
        return;
    }
    const bool includeHiddenWidgets = normalizedArgument == QStringLiteral("all");

    const QString noWidgetsMessage =
        trMessage(QStringLiteral("top-level QWidget はありません。"),
                  QStringLiteral("No top-level widgets."));

    QString sourceCode = QStringLiteral(R"(#include <QApplication>
#include <QDebug>
#include <QList>
#include <QWidget>
([]() {
const QWidgetList widgets = QApplication::topLevelWidgets();
QList<QWidget*> userWidgets;
for (QWidget* widget : widgets) {
    if (widget == nullptr || widget->property("_icppInspector").toBool()) {
        continue;
    }
    if (!%INCLUDE_HIDDEN% && !widget->isVisible()) {
        continue;
    }
    userWidgets.append(widget);
}

if (userWidgets.isEmpty()) {
    qDebug().noquote() << "%NO_WIDGETS%";
    return;
}

for (QWidget* widget : userWidgets) {
    qDebug().noquote()
        << QString("0x%1 %2 \"%3\" visible=%4")
               .arg(reinterpret_cast<quintptr>(widget), 0, 16)
               .arg(widget->metaObject()->className())
               .arg(widget->windowTitle())
               .arg(widget->isVisible() ? "true" : "false");
}
})();)");
    sourceCode.replace(QStringLiteral("%INCLUDE_HIDDEN%"),
                       includeHiddenWidgets ? QStringLiteral("true") : QStringLiteral("false"));
    sourceCode.replace(QStringLiteral("%NO_WIDGETS%"), noWidgetsMessage);
    interpreter.sendSource(sourceCode);
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::closeAllWidgets(const QString& historyEntry)
{
    if (!requireQtClingCommand(QStringLiteral(".closeall"))) {
        return;
    }

    QString resultExpression =
        helpLanguage == HelpLanguage::Japanese
            ? QStringLiteral(R"(QString("%1 個の widget を閉じました。").arg(closedCount))")
            : QStringLiteral(R"(QString("Closed %1 widget%2.").arg(closedCount).arg(closedCount == 1 ? "" : "s"))");
    QString sourceCode = QStringLiteral(R"(#include <QApplication>
#include <QDebug>
#include <QWidget>
([]() {
const QWidgetList widgets = QApplication::topLevelWidgets();
int closedCount = 0;
for (QWidget* widget : widgets) {
    if (widget != nullptr && widget->isVisible() && !widget->property("_icppInspector").toBool()) {
        widget->close();
        ++closedCount;
    }
}
qDebug().noquote() << %RESULT_EXPRESSION%;
})();)");
    sourceCode.replace(QStringLiteral("%RESULT_EXPRESSION%"), resultExpression);
    interpreter.sendSource(sourceCode);
    addHistoryEntry(historyEntry);
}

} // namespace icpp
