#include "MainWindow.h"
#include "BoardDocumentExchange.h"

#include "CanvasWidget.h"
#include "CenteredComboBox.h"
#include "FloatingActionChildButton.h"
#include "ImageExportDialog.h"
#include "ImageExporter.h"
#include "ImageImporter.h"
#include "PersistentToolBar.h"
#include "PlatformColorDialog.h"
#include "WhiteboardMcpController.h"
#include "WhiteboardMcpHttpServer.h"

#include <QActionGroup>
#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDir>
#include <QEasingCurve>
#include <QEvent>
#include <QFontDatabase>
#include <QFontDialog>
#include <QFontInfo>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPoint>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <QVariant>

#include <algorithm>

namespace {
bool isPlainTextEditRelated(QObject *object)
{
    for (QObject *current = object; current; current = current->parent()) {
        if (qobject_cast<QPlainTextEdit *>(current))
            return true;
    }
    return false;
}

bool toolForShortcutKey(int key, Tool *tool)
{
    switch (key) {
    case Qt::Key_1:
        *tool = Tool::Select;
        return true;
    case Qt::Key_2:
        *tool = Tool::Pen;
        return true;
    case Qt::Key_3:
        *tool = Tool::Eraser;
        return true;
    case Qt::Key_4:
        *tool = Tool::Line;
        return true;
    case Qt::Key_5:
        *tool = Tool::Rectangle;
        return true;
    case Qt::Key_6:
        *tool = Tool::Circle;
        return true;
    case Qt::Key_7:
        *tool = Tool::Text;
        return true;
    case Qt::Key_8:
        *tool = Tool::Polyline;
        return true;
    case Qt::Key_9:
        *tool = Tool::Bezier;
        return true;
    case Qt::Key_0:
        *tool = Tool::Arc;
        return true;
    default:
        return false;
    }
}

QImage imageFromMimeData(const QMimeData *mimeData)
{
    if (!mimeData || !mimeData->hasImage())
        return QImage();
    const QVariant imageData = mimeData->imageData();
    if (imageData.canConvert<QImage>())
        return imageData.value<QImage>();
    if (imageData.canConvert<QPixmap>())
        return imageData.value<QPixmap>().toImage();
    return QImage();
}

bool supportsFillControl(ElementType type, bool closed)
{
    if (type == ElementType::Rectangle || type == ElementType::RoundedRectangle || type == ElementType::Ellipse || type == ElementType::Circle)
        return true;
    if (type == ElementType::Polyline || type == ElementType::Bezier)
        return closed;
    return false;
}

bool supportsArrowControls(ElementType type, bool closed)
{
    if (type == ElementType::Line || type == ElementType::Arc)
        return true;
    if (type == ElementType::Polyline || type == ElementType::Bezier)
        return !closed;
    return false;
}

bool isDialogSelectableFontFamily(const QStringList &families, const QString &family)
{
    return !family.isEmpty() && !family.startsWith(QLatin1Char('.')) && families.contains(family);
}

void fitComboBoxPopupToContents(QComboBox *comboBox)
{
    if (!comboBox || !comboBox->view())
        return;
    int popupWidth = comboBox->width();
    const QFontMetrics metrics(comboBox->font());
    for (int i = 0; i < comboBox->count(); ++i)
        popupWidth = qMax(popupWidth, metrics.horizontalAdvance(comboBox->itemText(i)) + 48);
    comboBox->view()->setMinimumWidth(popupWidth);
}

QFont fontForDialog(const QFont &font)
{
    QFont dialogFont = font;
    const QStringList families = QFontDatabase::families();
    const QFontInfo resolvedFont(dialogFont);
    if (isDialogSelectableFontFamily(families, dialogFont.family()))
        return dialogFont;
    if (isDialogSelectableFontFamily(families, resolvedFont.family())) {
        dialogFont.setFamily(resolvedFont.family());
        return dialogFont;
    }

    QStringList fallbackFamilies = QFont::substitutes(dialogFont.family());
    fallbackFamilies.append(dialogFont.defaultFamily());
    fallbackFamilies.append(QFontDatabase::systemFont(QFontDatabase::GeneralFont).family());
    fallbackFamilies.append(QFontDatabase::systemFont(QFontDatabase::TitleFont).family());
    fallbackFamilies.append({
        QStringLiteral("Helvetica Neue"),
        QStringLiteral("Helvetica"),
        QStringLiteral("Hiragino Sans"),
        QStringLiteral("Arial"),
        QStringLiteral("Segoe UI"),
        QStringLiteral("Noto Sans"),
        QStringLiteral("DejaVu Sans"),
    });
    for (const QString &family : std::as_const(fallbackFamilies)) {
        if (isDialogSelectableFontFamily(families, family)) {
            dialogFont.setFamily(family);
            break;
        }
    }
    if (dialogFont.pointSize() <= 0 && resolvedFont.pointSize() > 0)
        dialogFont.setPointSize(resolvedFont.pointSize());
    return dialogFont;
}

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Whiteboard"));
    m_toolHelpDelayTimer = new QTimer(this);
    m_toolHelpDelayTimer->setSingleShot(true);
    m_toolHelpHideTimer = new QTimer(this);
    m_toolHelpHideTimer->setSingleShot(true);
    connect(m_toolHelpDelayTimer, &QTimer::timeout, this, [this]() {
        if (!m_toolHelpWidget)
            return;
        const QString text = m_toolHelpWidget->property("delayedToolHelp").toString();
        if (text.isEmpty())
            return;
        QToolTip::showText(m_toolHelpWidget->mapToGlobal(m_toolHelpWidget->rect().bottomLeft()), text, m_toolHelpWidget, QRect(), 3000);
        m_toolHelpHideTimer->start(3000);
    });
    connect(m_toolHelpHideTimer, &QTimer::timeout, this, []() { QToolTip::hideText(); });
    loadDocument();
    resize(m_model.settings().windowSize());
    m_floatingActionDockY = m_model.settings().floatingActionDockY();

    m_canvas = new CanvasWidget(this);
    m_canvas->setModel(&m_model);
    m_canvas->setChangeCallback([this]() { updateUiState(); });
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this]() { updateUiState(); });

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidget(m_canvas);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setStyleSheet(QStringLiteral("QScrollArea { background: #f5f7f8; border: 0; }"));

    auto *centralWidget = new QWidget(this);
    centralWidget->installEventFilter(this);
    auto *centralLayout = new QGridLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(m_scrollArea, 0, 0);
    createFloatingActions(centralWidget);
    setCentralWidget(centralWidget);

    createFileMenu();
    m_editMenu = menuBar()->addMenu(tr("Edit"));
    qApp->installEventFilter(this);
    createHelpMenu();
    createToolbar();
    updateUiState();
    restoreCurrentPageScrollPosition();
}

bool MainWindow::startMcpServer()
{
    if (m_mcpController)
        return true;

    m_mcpController = new WhiteboardMcpController(
        &m_model,
        m_canvas,
        [this]() { handleMcpChange(); },
        [this](const QString &direction) { return navigatePage(direction); },
        [this](const QString &toolName, const QJsonObject &arguments, QString *denyReason) {
            return approveMcpRequest(toolName, arguments, denyReason);
        },
        this);

    if (m_mcpController->start())
        return true;

    delete m_mcpController;
    m_mcpController = nullptr;
    return false;
}

bool MainWindow::startMcpHttpServer()
{
    if (m_mcpHttpServer)
        return m_mcpHttpServer->port() != 0 || m_mcpHttpServer->start();

    m_mcpHttpServer = new WhiteboardMcpHttpServer(
        &m_model,
        m_canvas,
        [this]() { handleMcpChange(); },
        [this](const QString &direction) { return navigatePage(direction); },
        [this](const QString &toolName, const QJsonObject &arguments, QString *denyReason) {
            return approveMcpRequest(toolName, arguments, denyReason);
        },
        this);
    if (m_mcpHttpServer->start())
        return true;
    return false;
}

QString MainWindow::mcpHttpErrorString() const
{
    return m_mcpHttpServer ? m_mcpHttpServer->errorString() : QString();
}

void MainWindow::handleMcpChange()
{
    m_canvas->refreshSize();
    restoreCurrentPageScrollPosition();
    updateUiState();
    saveDocument();
}

bool MainWindow::navigatePage(const QString &direction)
{
    if (direction != QStringLiteral("next") && direction != QStringLiteral("previous"))
        return false;
    const int previousIndex = m_model.document().currentPageIndex();
    rememberCurrentPageScrollPosition();
    if (direction == QStringLiteral("next"))
        m_model.nextPage();
    else if (direction == QStringLiteral("previous"))
        m_model.previousPage();
    if (m_model.document().currentPageIndex() == previousIndex)
        return false;
    m_canvas->refreshSize();
    restoreCurrentPageScrollPosition();
    updateUiState();
    return true;
}

bool MainWindow::approveMcpRequest(const QString &toolName,
                                   const QJsonObject &arguments,
                                   QString *denyReason)
{
    if (toolName == QStringLiteral("whiteboard/diagram/apply")) {
        rememberCurrentPageScrollPosition();
        return true;
    }

    QString requestText;
    if (toolName == QStringLiteral("whiteboard/elements/update")) {
        const int elementCount = arguments.value(QStringLiteral("updates")).toArray().size();
        requestText = tr("Allow AI to update %1 objects on the current page?").arg(elementCount);
    } else if (toolName == QStringLiteral("whiteboard/elements/delete")) {
        const int elementCount = arguments.value(QStringLiteral("ids")).toArray().size();
        requestText = tr("Allow AI to delete %1 objects from the current page?").arg(elementCount);
    } else if (toolName == QStringLiteral("whiteboard/elements/apply")) {
        const int addCount = arguments.value(QStringLiteral("add")).toArray().size();
        const int updateCount = arguments.value(QStringLiteral("update")).toArray().size();
        const int deleteCount = arguments.value(QStringLiteral("delete")).toArray().size();
        requestText = tr("Allow AI to add %1, update %2, and delete %3 objects on the current page?")
                          .arg(addCount).arg(updateCount).arg(deleteCount);
    } else if (toolName == QStringLiteral("whiteboard/history/undo")) {
        requestText = tr("Allow AI to undo the most recent whiteboard change?");
    } else if (toolName == QStringLiteral("whiteboard/history/redo")) {
        requestText = tr("Allow AI to redo the most recently undone whiteboard change?");
    } else if (toolName == QStringLiteral("whiteboard/page/add")) {
        requestText = tr("Allow AI to add an empty page after the current page?");
    } else if (toolName == QStringLiteral("whiteboard/page/delete")) {
        requestText = tr("Allow AI to delete the current page (%1)?")
                          .arg(m_model.document().currentPageIndex() + 1);
    } else if (toolName == QStringLiteral("whiteboard/page/lock")) {
        requestText = arguments.value(QStringLiteral("locked")).toBool()
            ? tr("Allow AI to lock the current page?")
            : tr("Allow AI to unlock the current page?");
    } else if (toolName == QStringLiteral("whiteboard/image/save")) {
        const QString filePath = arguments.value(QStringLiteral("filePath")).toString();
        const QString format = arguments.value(QStringLiteral("format")).toString().toUpper();
        const bool overwrite = arguments.value(QStringLiteral("overwrite")).toBool();
        requestText = overwrite
            ? tr("Allow AI to save a %1 image to '%2' and overwrite an existing file?").arg(format, filePath)
            : tr("Allow AI to save a %1 image to '%2'?").arg(format, filePath);
    } else {
        if (denyReason)
            *denyReason = tr("This AI operation is not approved.");
        return false;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, tr("AI Diagram Request"), requestText, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        if (denyReason)
            *denyReason = tr("The AI operation was declined by the user.");
        return false;
    }
    rememberCurrentPageScrollPosition();
    return true;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == centralWidget() && event->type() == QEvent::Resize) {
        QTimer::singleShot(0, this, [this]() { updateFloatingActionDockGeometry(); });
        return false;
    }

    if (watched->property("delayedToolHelp").isValid()) {
        switch (event->type()) {
        case QEvent::Enter:
            m_toolHelpWidget = qobject_cast<QWidget *>(watched);
            m_toolHelpDelayTimer->start(2000);
            return false;
        case QEvent::Leave:
        case QEvent::MouseButtonPress:
            hideDelayedToolHelp();
            return false;
        case QEvent::ToolTip:
            return true;
        default:
            break;
        }
    }

    if ((event->type() != QEvent::KeyPress && event->type() != QEvent::ShortcutOverride) || QApplication::activeWindow() != this)
        return QMainWindow::eventFilter(watched, event);

    if (isPlainTextEditRelated(watched) || isPlainTextEditRelated(QApplication::focusObject()))
        return QMainWindow::eventFilter(watched, event);

    auto *keyEvent = static_cast<QKeyEvent *>(event);
    const Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
    const bool commandModifier = modifiers.testFlag(Qt::ControlModifier) || modifiers.testFlag(Qt::MetaModifier);
    const bool onlyCommandModifier = commandModifier && !modifiers.testFlag(Qt::ShiftModifier) && !modifiers.testFlag(Qt::AltModifier);
    const bool commandShiftModifier = commandModifier && modifiers.testFlag(Qt::ShiftModifier) && !modifiers.testFlag(Qt::AltModifier);

    Tool shortcutTool = Tool::Select;
    const bool toolShortcut = onlyCommandModifier && toolForShortcutKey(keyEvent->key(), &shortcutTool);
    const bool sendBackwardShortcut = onlyCommandModifier && keyEvent->key() == Qt::Key_BracketLeft;
    const bool bringForwardShortcut = onlyCommandModifier && keyEvent->key() == Qt::Key_BracketRight;
    const bool sendToBackShortcut = commandShiftModifier && keyEvent->key() == Qt::Key_BracketLeft;
    const bool bringToFrontShortcut = commandShiftModifier && keyEvent->key() == Qt::Key_BracketRight;
    const bool groupShortcut = onlyCommandModifier && keyEvent->key() == Qt::Key_G;
    const bool ungroupShortcut = commandShiftModifier && keyEvent->key() == Qt::Key_G;
    const bool addPageShortcut = commandShiftModifier && keyEvent->key() == Qt::Key_N;
    const bool deletePageShortcut = commandShiftModifier && keyEvent->key() == Qt::Key_Backspace;
    if (toolShortcut || sendBackwardShortcut || bringForwardShortcut || sendToBackShortcut || bringToFrontShortcut || groupShortcut || ungroupShortcut || addPageShortcut || deletePageShortcut) {
        if (event->type() == QEvent::ShortcutOverride) {
            event->accept();
            return true;
        }

        if (toolShortcut) {
            if (m_model.currentPageLocked() && shortcutTool != Tool::Select)
                return true;
            m_model.selectTool(shortcutTool);
            updateUiState();
            return true;
        }
        if (sendBackwardShortcut) {
            if (m_model.moveSelectedBackward()) {
                m_canvas->update();
                updateUiState();
            }
            return true;
        }
        if (bringForwardShortcut) {
            if (m_model.moveSelectedForward()) {
                m_canvas->update();
                updateUiState();
            }
            return true;
        }
        if (sendToBackShortcut) {
            if (m_model.moveSelectedToBack()) {
                m_canvas->update();
                updateUiState();
            }
            return true;
        }
        if (bringToFrontShortcut) {
            if (m_model.moveSelectedToFront()) {
                m_canvas->update();
                updateUiState();
            }
            return true;
        }
        if (groupShortcut) {
            if (m_model.groupSelectedElements()) {
                m_canvas->update();
                updateUiState();
            }
            return true;
        }
        if (ungroupShortcut) {
            if (m_model.ungroupSelectedElements()) {
                m_canvas->update();
                updateUiState();
            }
            return true;
        }
        if (addPageShortcut) {
            rememberCurrentPageScrollPosition();
            if (m_model.addPage()) {
                m_canvas->refreshSize();
                restoreCurrentPageScrollPosition();
                updateUiState();
            }
            return true;
        }
        if (deletePageShortcut) {
            if (m_model.canDeletePage()) {
                const QMessageBox::StandardButton answer = QMessageBox::question(this, tr("Delete Page"), tr("Delete this page?"));
                if (answer == QMessageBox::Yes) {
                    rememberCurrentPageScrollPosition();
                    m_model.deleteCurrentPage();
                    m_canvas->refreshSize();
                    restoreCurrentPageScrollPosition();
                    updateUiState();
                }
            }
            return true;
        }
    }

    const bool closeByShortcut = commandModifier && modifiers.testFlag(Qt::ShiftModifier) && keyEvent->key() == Qt::Key_C;
    const bool openByShortcut = commandModifier && modifiers.testFlag(Qt::ShiftModifier) && keyEvent->key() == Qt::Key_O;
    if (!closeByShortcut && !openByShortcut)
        return QMainWindow::eventFilter(watched, event);

    if (event->type() == QEvent::ShortcutOverride) {
        event->accept();
        return true;
    }

    if (openByShortcut) {
        if (m_model.openSelectedPolyline() || m_model.openSelectedBezier()) {
            m_canvas->update();
            updateUiState();
            return true;
        }
        return QMainWindow::eventFilter(watched, event);
    }

    if (m_canvas->closeActivePolyline()) {
        updateUiState();
        return true;
    }
    if (m_canvas->closeActiveBezier()) {
        updateUiState();
        return true;
    }
    if (m_model.closeSelectedPolyline() || m_model.closeSelectedBezier()) {
        m_canvas->update();
        updateUiState();
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    rememberCurrentPageScrollPosition();
    m_model.setWindowSize(size());
    m_model.setFloatingActionDockY(m_floatingActionDockY);
    saveDocument();
    QMainWindow::closeEvent(event);
}

QAction *MainWindow::addToolAction(const QString &iconName, const QString &text, Tool tool)
{
    QAction *action = new QAction(QIcon(iconPath(iconName)), text, this);
    action->setCheckable(true);
    action->setData(static_cast<int>(tool));
    switch (tool) {
    case Tool::Select:
        action->setShortcuts({QKeySequence(QStringLiteral("Ctrl+1")), QKeySequence(QStringLiteral("Meta+1"))});
        break;
    case Tool::Pen:
        action->setShortcuts({QKeySequence(QStringLiteral("Ctrl+2")), QKeySequence(QStringLiteral("Meta+2"))});
        break;
    case Tool::Eraser:
        action->setShortcuts({QKeySequence(QStringLiteral("Ctrl+3")), QKeySequence(QStringLiteral("Meta+3"))});
        break;
    case Tool::Line:
        action->setShortcuts({QKeySequence(QStringLiteral("Ctrl+4")), QKeySequence(QStringLiteral("Meta+4"))});
        break;
    case Tool::Rectangle:
        action->setShortcuts({QKeySequence(QStringLiteral("Ctrl+5")), QKeySequence(QStringLiteral("Meta+5"))});
        break;
    case Tool::RoundedRectangle:
        break;
    case Tool::Ellipse:
        break;
    case Tool::Bezier:
        action->setShortcuts({QKeySequence(QStringLiteral("Ctrl+9")), QKeySequence(QStringLiteral("Meta+9"))});
        break;
    case Tool::Arc:
        action->setShortcuts({QKeySequence(QStringLiteral("Ctrl+0")), QKeySequence(QStringLiteral("Meta+0"))});
        break;
    case Tool::Circle:
        action->setShortcuts({QKeySequence(QStringLiteral("Ctrl+6")), QKeySequence(QStringLiteral("Meta+6"))});
        break;
    case Tool::Polyline:
        action->setShortcuts({QKeySequence(QStringLiteral("Ctrl+8")), QKeySequence(QStringLiteral("Meta+8"))});
        break;
    case Tool::Text:
        action->setShortcuts({QKeySequence(QStringLiteral("Ctrl+7")), QKeySequence(QStringLiteral("Meta+7"))});
        break;
    }
    action->setShortcutVisibleInContextMenu(true);
    connect(action, &QAction::triggered, this, [this, tool]() {
        m_model.selectTool(tool);
        updateUiState();
    });
    m_toolActions.append(action);
    return action;
}

QAction *MainWindow::addActionButton(const QString &iconName, const QString &text, const std::function<void()> &handler)
{
    QAction *action = new QAction(QIcon(iconPath(iconName)), text, this);
    connect(action, &QAction::triggered, this, [handler]() { handler(); });
    return action;
}

FloatingActionChildButton *MainWindow::addFloatingActionButton(QWidget *parent,
                                                               QWidget *labelParent,
                                                               const QString &iconName,
                                                               const QString &text,
                                                               const std::function<void()> &handler)
{
    auto *button = new FloatingActionChildButton(parent, labelParent);
    button->setIcon(QIcon(iconPath(iconName)));
    button->setIconSize(QSize(20, 20));
    button->setAccessibleName(text);
    button->setBubbleText(text);
    button->setAutoRaise(false);
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedSize(40, 40);
    connect(button, &QToolButton::clicked, this, [handler]() { handler(); });
    return button;
}

QComboBox *MainWindow::createArrowComboBox(const QString &labelPrefix)
{
    auto *comboBox = new CenteredComboBox(this);
    comboBox->addItem(labelPrefix + tr("None"), static_cast<int>(ArrowHead::None));
    comboBox->addItem(labelPrefix + tr("Triangle"), static_cast<int>(ArrowHead::Triangle));
    comboBox->addItem(labelPrefix + tr("Open"), static_cast<int>(ArrowHead::Open));
    comboBox->addItem(labelPrefix + tr("Diamond"), static_cast<int>(ArrowHead::Diamond));
    comboBox->setFixedWidth(82);
    comboBox->setFixedHeight(44);
    fitComboBoxPopupToContents(comboBox);
    connect(comboBox, &QComboBox::currentIndexChanged, this, [this]() { applyArrowHeadSettings(); });
    return comboBox;
}

QWidget *MainWindow::createSteppedSpinBox(QSpinBox *spinBox, int valueWidth)
{
    auto *container = new QWidget(this);
    container->setObjectName(QStringLiteral("toolbarSpinBoxStepper"));
    auto *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *decrementButton = new QToolButton(container);
    decrementButton->setObjectName(QStringLiteral("spinBoxDecrementButton"));
    decrementButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    decrementButton->setToolTip(tr("Decrease"));
    decrementButton->setAutoRaise(false);
    decrementButton->setCursor(Qt::PointingHandCursor);
    decrementButton->setFixedSize(24, 36);
    connect(decrementButton, &QToolButton::clicked, this, [spinBox]() {
        if (spinBox->isEnabled())
            spinBox->stepDown();
    });

    spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinBox->setFixedWidth(valueWidth);

    auto *incrementButton = new QToolButton(container);
    incrementButton->setObjectName(QStringLiteral("spinBoxIncrementButton"));
    incrementButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    incrementButton->setToolTip(tr("Increase"));
    incrementButton->setAutoRaise(false);
    incrementButton->setCursor(Qt::PointingHandCursor);
    incrementButton->setFixedSize(24, 36);
    connect(incrementButton, &QToolButton::clicked, this, [spinBox]() {
        if (spinBox->isEnabled())
            spinBox->stepUp();
    });

    layout->addWidget(decrementButton);
    layout->addWidget(spinBox);
    layout->addWidget(incrementButton);
    updateSteppedSpinBoxButtons(spinBox);
    return container;
}

void MainWindow::registerShortcutAction(QAction *action)
{
    action->setShortcutContext(Qt::ApplicationShortcut);
    action->setShortcutVisibleInContextMenu(true);
    addAction(action);
}

void MainWindow::registerDelayedToolHelp(QWidget *widget, const QString &text)
{
    if (!widget)
        return;
    widget->setProperty("delayedToolHelp", text);
    widget->setToolTip(QString());
    widget->installEventFilter(this);
}

void MainWindow::hideDelayedToolHelp()
{
    if (m_toolHelpDelayTimer)
        m_toolHelpDelayTimer->stop();
    if (m_toolHelpHideTimer)
        m_toolHelpHideTimer->stop();
    m_toolHelpWidget = nullptr;
    QToolTip::hideText();
}

QString MainWindow::iconPath(const QString &iconName) const
{
    return QStringLiteral(":/assets/icons/%1.svg").arg(iconName);
}

QString MainWindow::saveFilePath() const
{
    QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (directory.isEmpty())
        directory = QDir::homePath() + QStringLiteral("/.whiteboard-app");
    QDir().mkpath(directory);
    return directory + QStringLiteral("/whiteboard.json");
}

void MainWindow::createFileMenu()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    m_drawingImportAction = fileMenu->addAction(tr("Import Drawing Data..."));
    m_drawingImportAction->setObjectName(QStringLiteral("importDrawingData"));
    connect(m_drawingImportAction, &QAction::triggered, this, &MainWindow::performDrawingImport);
    QAction *drawingExportAction = fileMenu->addAction(tr("Export Drawing Data..."));
    drawingExportAction->setObjectName(QStringLiteral("exportDrawingData"));
    connect(drawingExportAction, &QAction::triggered, this, &MainWindow::performDrawingExport);
    fileMenu->addSeparator();
    m_imageImportAction = new QAction(tr("Import Image..."), this);
    connect(m_imageImportAction, &QAction::triggered, this, [this]() { performImageImport(); });
    fileMenu->addAction(m_imageImportAction);

    m_imageExportAction = new QAction(tr("Save Image..."), this);
    connect(m_imageExportAction, &QAction::triggered, this, [this]() { performImageExport(); });
    fileMenu->addAction(m_imageExportAction);
}

void MainWindow::createHelpMenu()
{
    QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
    QAction *shortcutHelpAction = new QAction(tr("Shortcuts"), this);
    shortcutHelpAction->setShortcut(QKeySequence::HelpContents);
    connect(shortcutHelpAction, &QAction::triggered, this, [this]() { showShortcutHelp(); });
    helpMenu->addAction(shortcutHelpAction);
}

void MainWindow::createToolbar()
{
    QToolBar *toolbar = addToolBar(tr("Tools"));
    m_toolbar = toolbar;
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));
    toolbar->setStyleSheet(QStringLiteral(
        "QToolBar {"
        "background: #3f464d;"
        "border: 0;"
        "border-bottom: 1px solid #242a30;"
        "spacing: 5px;"
        "padding: 8px 10px;"
        "}"
        "QToolBar::separator {"
        "background: #68737d;"
        "width: 1px;"
        "margin: 5px 7px;"
        "}"
        "QFrame#persistentToolbarSeparator {"
        "color: #68737d;"
        "margin: 5px 7px;"
        "}"
        "QToolButton {"
        "border: 1px solid #9aa6af;"
        "border-radius: 6px;"
        "padding: 6px;"
        "background: #ffffff;"
        "}"
        "QToolButton:hover { background: #fdfefe; border-color: #b9c8ce; }"
        "QToolButton[toolbarRole=\"tool\"] { background: #bae6fd; border-color: #38bdf8; }"
        "QToolButton[toolbarRole=\"shape\"] { background: #bbf7d0; border-color: #4ade80; }"
        "QToolButton[toolbarRole=\"text\"] { background: #fde68a; border-color: #eab308; }"
        "QToolButton[toolbarRole=\"edit\"] { background: #bfdbfe; border-color: #60a5fa; }"
        "QToolButton[toolbarRole=\"style\"] { background: #fed7aa; border-color: #fb923c; }"
        "QToolButton[toolbarRole=\"zoom\"] { background: #cffafe; border-color: #67e8f9; }"
        "QToolButton[toolbarRole=\"page\"] { background: #fecdd3; border-color: #fb7185; }"
        "QToolButton[toolbarRole=\"danger\"] { background: #fb7185; border-color: #e11d48; }"
        "QToolButton[toolbarRole=\"tool\"]:hover { background: #7dd3fc; border-color: #0284c7; }"
        "QToolButton[toolbarRole=\"shape\"]:hover { background: #86efac; border-color: #22c55e; }"
        "QToolButton[toolbarRole=\"text\"]:hover { background: #facc15; border-color: #ca8a04; }"
        "QToolButton[toolbarRole=\"edit\"]:hover { background: #93c5fd; border-color: #3b82f6; }"
        "QToolButton[toolbarRole=\"style\"]:hover { background: #fdba74; border-color: #f97316; }"
        "QToolButton[toolbarRole=\"zoom\"]:hover { background: #a5f3fc; border-color: #22d3ee; }"
        "QToolButton[toolbarRole=\"page\"]:hover { background: #fda4af; border-color: #f43f5e; }"
        "QToolButton[toolbarRole=\"danger\"]:hover { background: #f43f5e; border-color: #be123c; }"
        "QToolButton:checked { background: #e6f4f1; border-color: #75c5b7; }"
        "QToolButton[toolbarRole=\"tool\"]:checked { background: #38bdf8; border-color: #0369a1; }"
        "QToolButton[toolbarRole=\"shape\"]:checked { background: #34d399; border-color: #047857; }"
        "QToolButton[toolbarRole=\"text\"]:checked { background: #facc15; border-color: #a16207; }"
        "QToolButton:pressed { background: #dcefeb; }"
        "QToolButton:disabled { color: #6f7a84; background: #d4dae0; border-color: #87919b; }"
        "QToolButton[toolbarRole=\"tool\"]:disabled { background: #9fd2e8; border-color: #6eaec8; }"
        "QToolButton[toolbarRole=\"shape\"]:disabled { background: #a5e8b8; border-color: #71c78b; }"
        "QToolButton[toolbarRole=\"text\"]:disabled { background: #efd77b; border-color: #c9aa32; }"
        "QToolButton[toolbarRole=\"edit\"]:disabled { background: #a9cdfa; border-color: #7fb2ed; }"
        "QToolButton[toolbarRole=\"style\"]:disabled { background: #efb47d; border-color: #c9874d; }"
        "QToolButton[toolbarRole=\"zoom\"]:disabled { background: #9eeaf3; border-color: #69c6d0; }"
        "QToolButton[toolbarRole=\"page\"]:disabled { background: #e89aa6; border-color: #c96d7b; }"
        "QToolButton[toolbarRole=\"danger\"]:disabled { background: #e78392; border-color: #bd5f6d; }"
        "QSpinBox, QComboBox {"
        "background: #f8fafc;"
        "border: 1px solid #9aa6af;"
        "border-radius: 6px;"
        "min-height: 34px;"
        "padding: 0 4px;"
        "color: #344054;"
        "selection-background-color: #c7e7e2;"
        "}"
        "QSpinBox:hover, QComboBox:hover { background: #ffffff; border-color: #7f8b94; }"
        "QSpinBox:focus, QComboBox:focus { border: 1px solid #0f766e; background: #ffffff; }"
        "QSpinBox:disabled, QComboBox:disabled { background: #e5e7eb; color: #94a3b8; border-color: #cbd5e1; }"
        "QComboBox::drop-down, QSpinBox::up-button, QSpinBox::down-button {"
        "border: 0;"
        "background: transparent;"
        "width: 8px;"
        "}"
        "QWidget#toolbarSpinBoxStepper {"
        "background: #f8fafc;"
        "border: 1px solid #9aa6af;"
        "border-radius: 6px;"
        "}"
        "QWidget#toolbarSpinBoxStepper:hover { background: #ffffff; border-color: #7f8b94; }"
        "QWidget#toolbarSpinBoxStepper QSpinBox {"
        "background: transparent;"
        "border: 0;"
        "border-radius: 0;"
        "min-height: 34px;"
        "padding: 0 2px;"
        "}"
        "QWidget#toolbarSpinBoxStepper QToolButton {"
        "background: transparent;"
        "border: 0;"
        "border-radius: 0;"
        "padding: 0;"
        "}"
        "QWidget#toolbarSpinBoxStepper QToolButton:hover { background: #dbe7ec; }"
        "QWidget#toolbarSpinBoxStepper QToolButton:pressed { background: #c7d6de; }"
        "QWidget#toolbarSpinBoxStepper QToolButton:disabled { background: transparent; }"
        "QWidget#toolbarSpinBoxStepper QToolButton#spinBoxDecrementButton {"
        "border-top-left-radius: 5px;"
        "border-bottom-left-radius: 5px;"
        "}"
        "QWidget#toolbarSpinBoxStepper QToolButton#spinBoxIncrementButton {"
        "border-top-right-radius: 5px;"
        "border-bottom-right-radius: 5px;"
        "}"));
    auto *toolbarShadow = new QGraphicsDropShadowEffect(toolbar);
    toolbarShadow->setBlurRadius(10.0);
    toolbarShadow->setColor(QColor(16, 24, 40, 48));
    toolbarShadow->setOffset(0.0, 1.0);
    toolbar->setGraphicsEffect(toolbarShadow);

    m_persistentToolbar = new PersistentToolBar(this, toolbar);
    m_persistentToolbar->setIconSize(toolbar->iconSize());
    m_persistentToolbar->setOverflowStyleSheet(toolbar->styleSheet());
    toolbar->addWidget(m_persistentToolbar);

    createToolButtons(m_persistentToolbar);
    createEditActions(m_persistentToolbar);
    m_persistentToolbar->addSeparator();
    createStyleControls(m_persistentToolbar);
    m_persistentToolbar->addSeparator();
    createZoomControls(m_persistentToolbar);
    m_persistentToolbar->addSeparator();
    createPageControls(m_persistentToolbar);
    m_persistentToolbar->refreshOverflow();
}

void MainWindow::createFloatingActions(QWidget *parent)
{
    auto *panel = new QFrame(parent);
    m_floatingActionPanel = panel;
    panel->setObjectName(QStringLiteral("floatingActionPanel"));
    panel->setStyleSheet(QStringLiteral(
        "QFrame#floatingActionPanel {"
        "background: transparent;"
        "border: 0;"
        "}"
        "QFrame#floatingActionPanel QToolButton {"
        "background: #ffffff;"
        "border: 1px solid #d7e3e7;"
        "border-radius: 20px;"
        "padding: 0;"
        "}"
        "QFrame#floatingActionPanel QToolButton#floatingMainButton {"
        "background: #0f766e;"
        "border: 1px solid #0f766e;"
        "border-radius: 24px;"
        "}"
        "QFrame#floatingActionPanel QToolButton#floatingSelectButton { background: #bae6fd; border-color: #38bdf8; }"
        "QFrame#floatingActionPanel QToolButton#floatingResetButton { background: #fed7aa; border-color: #fb923c; }"
        "QFrame#floatingActionPanel QToolButton#floatingUndoButton,"
        "QFrame#floatingActionPanel QToolButton#floatingRedoButton { background: #60a5fa; border-color: #2563eb; }"
        "QFrame#floatingActionPanel QToolButton#floatingCopyButton,"
        "QFrame#floatingActionPanel QToolButton#floatingPasteButton { background: #34d399; border-color: #059669; }"
        "QFrame#floatingActionPanel QToolButton#floatingMainButton:hover { background: #115e59; border-color: #115e59; }"
        "QFrame#floatingActionPanel QToolButton:hover { background: #e6f4f1; border-color: #75c5b7; }"
        "QFrame#floatingActionPanel QToolButton#floatingSelectButton:hover { background: #7dd3fc; border-color: #0284c7; }"
        "QFrame#floatingActionPanel QToolButton#floatingResetButton:hover { background: #fdba74; border-color: #f97316; }"
        "QFrame#floatingActionPanel QToolButton#floatingUndoButton:hover,"
        "QFrame#floatingActionPanel QToolButton#floatingRedoButton:hover { background: #3b82f6; border-color: #1d4ed8; }"
        "QFrame#floatingActionPanel QToolButton#floatingCopyButton:hover,"
        "QFrame#floatingActionPanel QToolButton#floatingPasteButton:hover { background: #10b981; border-color: #047857; }"
        "QFrame#floatingActionPanel QToolButton:pressed { background: #d8eee9; }"
        "QFrame#floatingActionPanel QToolButton:disabled { background: #f2f4f7; border-color: #e4e7ec; }"));
    m_floatingActionContainer = new QWidget(panel);
    auto *actionLayout = new QVBoxLayout(m_floatingActionContainer);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(8);
    actionLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    m_floatingSelectButton = addFloatingActionButton(m_floatingActionContainer, parent, QStringLiteral("select"), tr("Select"), [this]() {
        m_model.selectTool(Tool::Select);
        updateUiState();
    });
    m_floatingSelectButton->setObjectName(QStringLiteral("floatingSelectButton"));
    m_floatingResetButton = addFloatingActionButton(m_floatingActionContainer, parent, QStringLiteral("reset-style"), tr("Reset Style"), [this]() {
        performResetStyle();
    });
    m_floatingResetButton->setObjectName(QStringLiteral("floatingResetButton"));
    m_floatingUndoButton = addFloatingActionButton(m_floatingActionContainer, parent, QStringLiteral("undo"), tr("Undo"), [this]() {
        performUndo();
    });
    m_floatingUndoButton->setObjectName(QStringLiteral("floatingUndoButton"));
    m_floatingRedoButton = addFloatingActionButton(m_floatingActionContainer, parent, QStringLiteral("redo"), tr("Redo"), [this]() {
        performRedo();
    });
    m_floatingRedoButton->setObjectName(QStringLiteral("floatingRedoButton"));
    m_floatingCopyButton = addFloatingActionButton(m_floatingActionContainer, parent, QStringLiteral("copy"), tr("Copy"), [this]() {
        performCopy();
    });
    m_floatingCopyButton->setObjectName(QStringLiteral("floatingCopyButton"));
    m_floatingPasteButton = addFloatingActionButton(m_floatingActionContainer, parent, QStringLiteral("paste"), tr("Paste"), [this]() {
        performPaste();
    });
    m_floatingPasteButton->setObjectName(QStringLiteral("floatingPasteButton"));
    m_floatingMainButton = new FloatingActionButton(panel);
    m_floatingMainButton->setIcon(QIcon(iconPath(QStringLiteral("quick-actions"))));
    m_floatingMainButton->setIconSize(QSize(20, 20));
    m_floatingMainButton->setToolTip(tr("Quick Actions"));
    m_floatingMainButton->setAccessibleName(tr("Quick Actions"));
    m_floatingMainButton->setAutoRaise(false);
    m_floatingMainButton->setCursor(Qt::PointingHandCursor);
    m_floatingMainButton->setObjectName(QStringLiteral("floatingMainButton"));
    m_floatingMainButton->setFixedSize(48, 48);
    connect(m_floatingMainButton, &FloatingActionButton::activated, this, [this]() {
        setFloatingActionsExpanded(!m_floatingActionsExpanded);
    });
    connect(m_floatingMainButton, &FloatingActionButton::draggedTo, this, [this](int topY) {
        constexpr int verticalMargin = 24;
        QWidget *dockParent = m_floatingActionPanel ? m_floatingActionPanel->parentWidget() : nullptr;
        const int maxDockY = dockParent
            ? qMax(verticalMargin, dockParent->height() - m_floatingMainButton->height() - verticalMargin)
            : verticalMargin;
        m_floatingActionDockY = qBound(verticalMargin, topY, maxDockY);
        updateFloatingActionButtonPositions();
    });

    actionLayout->addWidget(m_floatingSelectButton);
    actionLayout->addWidget(m_floatingResetButton);
    actionLayout->addWidget(m_floatingUndoButton);
    actionLayout->addWidget(m_floatingRedoButton);
    actionLayout->addWidget(m_floatingCopyButton);
    actionLayout->addWidget(m_floatingPasteButton);
    panel->setFixedWidth(64);
    panel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_floatingActionContainer->setGeometry((panel->width() - 40) / 2, 0, 40, 0);
    m_floatingActionsOpacityEffect = new QGraphicsOpacityEffect(m_floatingActionContainer);
    m_floatingActionsOpacityEffect->setOpacity(0.0);
    m_floatingActionContainer->setGraphicsEffect(m_floatingActionsOpacityEffect);
    updateFloatingActionDockGeometry();
    m_floatingActionsAnimation = new QPropertyAnimation(m_floatingActionContainer, "geometry", this);
    m_floatingActionsAnimation->setDuration(220);
    m_floatingActionsAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_floatingActionsAnimation, &QPropertyAnimation::valueChanged, this, [this]() { updateFloatingActionPanelMask(); });
    m_floatingActionsOpacityAnimation = new QPropertyAnimation(m_floatingActionsOpacityEffect, "opacity", this);
    m_floatingActionsOpacityAnimation->setDuration(180);
    m_floatingActionsOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    setFloatingActionsExpanded(false);

    panel->raise();
    QTimer::singleShot(0, this, [this]() { updateFloatingActionDockGeometry(); });
}

void MainWindow::createToolButtons(PersistentToolBar *toolbar)
{
    auto *toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);
    const auto addTool = [this, toolbar, toolGroup](const QString &iconName, const QString &text, Tool tool, const QString &role) {
        QAction *action = addToolAction(iconName, text, tool);
        toolGroup->addAction(action);
        toolbar->addAction(action);
        setToolbarButtonRole(toolbar, action, role);
        registerShortcutAction(action);
        registerDelayedToolHelp(toolbar->widgetForAction(action), action->text());
        return action;
    };

    addTool(QStringLiteral("select"), tr("Select"), Tool::Select, QStringLiteral("tool"));
    addTool(QStringLiteral("pen"), tr("Pen"), Tool::Pen, QStringLiteral("tool"));
    addTool(QStringLiteral("eraser"), tr("Eraser"), Tool::Eraser, QStringLiteral("tool"));

    toolbar->addSeparator();
    addTool(QStringLiteral("line"), tr("Line"), Tool::Line, QStringLiteral("shape"));
    addTool(QStringLiteral("rectangle"), tr("Rectangle"), Tool::Rectangle, QStringLiteral("shape"));
    addTool(QStringLiteral("rounded-rectangle"), tr("Rounded Rectangle"), Tool::RoundedRectangle, QStringLiteral("shape"));
    addTool(QStringLiteral("circle"), tr("Circle"), Tool::Circle, QStringLiteral("shape"));
    addTool(QStringLiteral("ellipse"), tr("Ellipse"), Tool::Ellipse, QStringLiteral("shape"));
    addTool(QStringLiteral("polyline"), tr("Polyline"), Tool::Polyline, QStringLiteral("shape"));
    addTool(QStringLiteral("bezier"), tr("Bezier"), Tool::Bezier, QStringLiteral("shape"));
    addTool(QStringLiteral("arc"), tr("Arc"), Tool::Arc, QStringLiteral("shape"));

    toolbar->addSeparator();
    addTool(QStringLiteral("text"), tr("Text"), Tool::Text, QStringLiteral("text"));
}

void MainWindow::createEditActions(PersistentToolBar *toolbar)
{
    m_fontAction = addActionButton(QStringLiteral("font"), tr("Font"), [this]() {
        if (m_model.hasSelection() && m_model.selectedElementType() != ElementType::Text)
            return;
        bool ok = false;
        const bool editingSelectedText = m_model.hasSelection() && m_model.selectedElementType() == ElementType::Text;
        const QFont initialFont = fontForDialog(editingSelectedText ? m_model.selectedElementFont() : m_model.settings().font());
        const QFont font = QFontDialog::getFont(&ok, initialFont, this, tr("Font"), QFontDialog::DontUseNativeDialog);
        if (!ok)
            return;
        if (editingSelectedText) {
            if (m_model.updateSelectedTextFont(font)) {
                m_canvas->update();
                updateUiState();
            }
        } else {
            m_model.setFont(font);
        }
    });
    toolbar->addAction(m_fontAction);
    setToolbarButtonRole(toolbar, m_fontAction, QStringLiteral("text"));

    toolbar->addSeparator();
    m_undoAction = addActionButton(QStringLiteral("undo"), tr("Undo"), [this]() { performUndo(); });
    m_undoAction->setShortcuts(QKeySequence::Undo);
    toolbar->addAction(m_undoAction);
    setToolbarButtonRole(toolbar, m_undoAction, QStringLiteral("edit"));
    registerShortcutAction(m_undoAction);

    m_redoAction = addActionButton(QStringLiteral("redo"), tr("Redo"), [this]() { performRedo(); });
    m_redoAction->setShortcuts({QKeySequence::Redo, QKeySequence(QStringLiteral("Ctrl+Shift+Z"))});
    toolbar->addAction(m_redoAction);
    setToolbarButtonRole(toolbar, m_redoAction, QStringLiteral("edit"));
    registerShortcutAction(m_redoAction);

    QAction *deleteSelectedAction = new QAction(tr("Delete Selected Object"), this);
    deleteSelectedAction->setShortcuts({
        QKeySequence(Qt::Key_Delete),
        QKeySequence(Qt::Key_Backspace),
    });
    connect(deleteSelectedAction, &QAction::triggered, this, [this]() { performDeleteSelected(); });
    registerShortcutAction(deleteSelectedAction);

    m_copyAction = new QAction(QIcon(iconPath(QStringLiteral("copy"))), tr("Copy"), this);
    m_copyAction->setShortcuts(QKeySequence::Copy);
    connect(m_copyAction, &QAction::triggered, this, [this]() { performCopy(); });
    toolbar->addAction(m_copyAction);
    setToolbarButtonRole(toolbar, m_copyAction, QStringLiteral("edit"));
    registerShortcutAction(m_copyAction);
    if (m_editMenu)
        m_editMenu->addAction(m_copyAction);

    m_pasteAction = new QAction(QIcon(iconPath(QStringLiteral("paste"))), tr("Paste"), this);
    m_pasteAction->setShortcuts(QKeySequence::Paste);
    connect(m_pasteAction, &QAction::triggered, this, [this]() { performPaste(); });
    toolbar->addAction(m_pasteAction);
    setToolbarButtonRole(toolbar, m_pasteAction, QStringLiteral("edit"));
    registerShortcutAction(m_pasteAction);
    if (m_editMenu)
        m_editMenu->addAction(m_pasteAction);

    m_groupAction = addActionButton(QStringLiteral("group"), tr("Group"), [this]() {
        if (m_model.groupSelectedElements()) {
            m_canvas->update();
            updateUiState();
        }
    });
    m_groupAction->setShortcuts({
        QKeySequence(QStringLiteral("Ctrl+G")),
        QKeySequence(QStringLiteral("Meta+G")),
    });
    toolbar->addAction(m_groupAction);
    setToolbarButtonRole(toolbar, m_groupAction, QStringLiteral("edit"));
    registerShortcutAction(m_groupAction);
    if (m_editMenu)
        m_editMenu->addAction(m_groupAction);

    m_ungroupAction = addActionButton(QStringLiteral("ungroup"), tr("Ungroup"), [this]() {
        if (m_model.ungroupSelectedElements()) {
            m_canvas->update();
            updateUiState();
        }
    });
    m_ungroupAction->setShortcuts({
        QKeySequence(QStringLiteral("Ctrl+Shift+G")),
        QKeySequence(QStringLiteral("Meta+Shift+G")),
    });
    toolbar->addAction(m_ungroupAction);
    setToolbarButtonRole(toolbar, m_ungroupAction, QStringLiteral("edit"));
    registerShortcutAction(m_ungroupAction);
    if (m_editMenu)
        m_editMenu->addAction(m_ungroupAction);

    m_sendBackwardAction = addActionButton(QStringLiteral("send-backward"), tr("Send Backward"), [this]() {
        if (m_model.moveSelectedBackward()) {
            m_canvas->update();
            updateUiState();
        }
    });
    m_sendBackwardAction->setShortcuts({
        QKeySequence(QStringLiteral("Ctrl+[")),
        QKeySequence(QStringLiteral("Meta+[")),
    });
    toolbar->addAction(m_sendBackwardAction);
    setToolbarButtonRole(toolbar, m_sendBackwardAction, QStringLiteral("edit"));
    registerShortcutAction(m_sendBackwardAction);
    if (m_editMenu)
        m_editMenu->addAction(m_sendBackwardAction);

    m_bringForwardAction = addActionButton(QStringLiteral("bring-forward"), tr("Bring Forward"), [this]() {
        if (m_model.moveSelectedForward()) {
            m_canvas->update();
            updateUiState();
        }
    });
    m_bringForwardAction->setShortcuts({
        QKeySequence(QStringLiteral("Ctrl+]")),
        QKeySequence(QStringLiteral("Meta+]")),
    });
    toolbar->addAction(m_bringForwardAction);
    setToolbarButtonRole(toolbar, m_bringForwardAction, QStringLiteral("edit"));
    registerShortcutAction(m_bringForwardAction);
    if (m_editMenu)
        m_editMenu->addAction(m_bringForwardAction);

    m_sendToBackAction = addActionButton(QStringLiteral("send-backward"), tr("Send to Back"), [this]() {
        if (m_model.moveSelectedToBack()) {
            m_canvas->update();
            updateUiState();
        }
    });
    m_sendToBackAction->setShortcuts({
        QKeySequence(QStringLiteral("Ctrl+Shift+[")),
        QKeySequence(QStringLiteral("Meta+Shift+[")),
    });
    registerShortcutAction(m_sendToBackAction);
    if (m_editMenu)
        m_editMenu->addAction(m_sendToBackAction);

    m_bringToFrontAction = addActionButton(QStringLiteral("bring-forward"), tr("Bring to Front"), [this]() {
        if (m_model.moveSelectedToFront()) {
            m_canvas->update();
            updateUiState();
        }
    });
    m_bringToFrontAction->setShortcuts({
        QKeySequence(QStringLiteral("Ctrl+Shift+]")),
        QKeySequence(QStringLiteral("Meta+Shift+]")),
    });
    registerShortcutAction(m_bringToFrontAction);
    if (m_editMenu)
        m_editMenu->addAction(m_bringToFrontAction);

    QAction *closePolylineAction = new QAction(tr("Close Polyline or Bezier"), this);
    closePolylineAction->setShortcuts({
        QKeySequence(QStringLiteral("Ctrl+Shift+C")),
    });
    connect(closePolylineAction, &QAction::triggered, this, [this]() {
        if (m_canvas->closeActivePolyline()) {
            updateUiState();
            return;
        }
        if (m_canvas->closeActiveBezier()) {
            updateUiState();
            return;
        }
        if (m_model.closeSelectedPolyline() || m_model.closeSelectedBezier()) {
            m_canvas->update();
            updateUiState();
        }
    });
    registerShortcutAction(closePolylineAction);
    if (m_editMenu)
        m_editMenu->addAction(closePolylineAction);

    QAction *openPolylineAction = new QAction(tr("Open Polyline or Bezier"), this);
    openPolylineAction->setShortcuts({
        QKeySequence(QStringLiteral("Ctrl+Shift+O")),
        QKeySequence(QStringLiteral("Meta+Shift+O")),
    });
    connect(openPolylineAction, &QAction::triggered, this, [this]() {
        if (m_model.openSelectedPolyline() || m_model.openSelectedBezier()) {
            m_canvas->update();
            updateUiState();
        }
    });
    registerShortcutAction(openPolylineAction);
    if (m_editMenu)
        m_editMenu->addAction(openPolylineAction);
}

void MainWindow::createStyleControls(PersistentToolBar *toolbar)
{
    m_colorAction = addActionButton(QStringLiteral("color"), tr("Color"), [this]() {
        const bool editingSelectedElement = m_model.hasSelection();
        const bool editingSelectedText = editingSelectedElement && m_model.selectedElementType() == ElementType::Text;
        const QColor initialColor = editingSelectedElement ? m_model.selectedElementColor() : m_model.settings().selectedColor();
        const QColor color = QColorDialog::getColor(initialColor, this, tr("Color"));
        if (!color.isValid())
            return;
        if (editingSelectedText) {
            if (m_model.updateSelectedTextColor(color)) {
                m_canvas->update();
                updateUiState();
            }
        } else if (editingSelectedElement) {
            if (m_model.updateSelectedColor(color)) {
                m_canvas->update();
                updateUiState();
            }
        } else {
            m_model.setSelectedColor(color);
        }
    });
    toolbar->addAction(m_colorAction);
    setToolbarButtonRole(toolbar, m_colorAction, QStringLiteral("style"));
    m_fillAction = addActionButton(QStringLiteral("fill-color"), tr("Fill"), [this]() {
        const QColor initialColor = m_model.hasSelection() ? m_model.selectedElementFillColor() : m_model.settings().fillColor();
        const QColor color = PlatformColorDialog::getFillColor(initialColor, this);
        if (!color.isValid())
            return;
        if (m_model.hasSelection() && m_model.updateSelectedFillColor(color)) {
            m_canvas->update();
            updateUiState();
            return;
        }
        m_model.setFillColor(color);
        updateUiState();
    });
    toolbar->addAction(m_fillAction);
    setToolbarButtonRole(toolbar, m_fillAction, QStringLiteral("style"));
    m_resetStyleAction = addActionButton(QStringLiteral("reset-style"), tr("Reset Style"), [this]() {
        performResetStyle();
    });
    toolbar->addAction(m_resetStyleAction);
    setToolbarButtonRole(toolbar, m_resetStyleAction, QStringLiteral("style"));
    m_scaleAction = addActionButton(QStringLiteral("scale"), tr("Scale Selected Shape"), [this]() {
        const bool singleTextSelected = m_model.selectedElementIndexes().size() <= 1 && m_model.selectedElementType() == ElementType::Text;
        if (!m_model.hasSelection() || singleTextSelected)
            return;
        bool ok = false;
        const int zoomPercent = m_model.settings().zoomPercent();
        const double percent = QInputDialog::getDouble(this, tr("Scale Selected Shape"), tr("Scale selected shape (%)"), 100.0, 1.0, 1000.0, 1, &ok);
        if (!ok)
            return;
        if (m_model.scaleSelectedBy(percent / 100.0)) {
            m_model.setZoomPercent(zoomPercent);
            m_canvas->update();
            updateUiState();
        }
    });
    m_scaleAction->setToolTip(tr("Scale only the selected shape"));
    toolbar->addAction(m_scaleAction);
    setToolbarButtonRole(toolbar, m_scaleAction, QStringLiteral("style"));

    m_strokeSpinBox = new QSpinBox(this);
    m_strokeSpinBox->setRange(0, 32);
    m_strokeSpinBox->setValue(m_model.settings().strokeWidth());
    m_strokeSpinBox->setPrefix(tr("Line "));
    connect(m_strokeSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        if (m_model.hasSelection() && m_model.selectedElementType() != ElementType::Text) {
            if (m_model.updateSelectedStrokeWidth(value)) {
                m_canvas->update();
                updateUiState();
            }
            return;
        }
        m_model.setStrokeWidth(value);
    });
    toolbar->addWidget(createSteppedSpinBox(m_strokeSpinBox, 58));

    m_cornerRadiusSpinBox = new QSpinBox(this);
    m_cornerRadiusSpinBox->setRange(0, 200);
    m_cornerRadiusSpinBox->setValue(m_model.settings().cornerRadius());
    m_cornerRadiusSpinBox->setPrefix(tr("Round "));
    connect(m_cornerRadiusSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        if (m_model.hasSelection() && m_model.selectedElementType() == ElementType::RoundedRectangle) {
            if (m_model.updateSelectedCornerRadius(value)) {
                m_canvas->update();
                updateUiState();
            }
            return;
        }
        if (m_model.hasSelection())
            return;
        m_model.setCornerRadius(value);
    });
    toolbar->addWidget(createSteppedSpinBox(m_cornerRadiusSpinBox, 72));

    m_strokeStyleComboBox = new CenteredComboBox(this);
    m_strokeStyleComboBox->addItem(tr("Solid"), static_cast<int>(StrokeStyle::Solid));
    m_strokeStyleComboBox->addItem(tr("Dotted"), static_cast<int>(StrokeStyle::Dotted));
    m_strokeStyleComboBox->setFixedWidth(58);
    m_strokeStyleComboBox->setFixedHeight(44);
    fitComboBoxPopupToContents(m_strokeStyleComboBox);
    connect(m_strokeStyleComboBox, &QComboBox::currentIndexChanged, this, [this]() { applyStrokeStyleSettings(); });
    toolbar->addWidget(m_strokeStyleComboBox);

    m_startArrowComboBox = createArrowComboBox(tr("Start: "));
    m_startArrowComboBox->setToolTip(tr("Start endpoint arrow"));
    toolbar->addWidget(m_startArrowComboBox);

    m_endArrowComboBox = createArrowComboBox(tr("End: "));
    m_endArrowComboBox->setToolTip(tr("End endpoint arrow"));
    toolbar->addWidget(m_endArrowComboBox);
}

void MainWindow::createZoomControls(PersistentToolBar *toolbar)
{
    QAction *zoomOutAction = addActionButton(QStringLiteral("zoom-out"), tr("Zoom Out"), [this]() { performZoomBy(-10); });
    zoomOutAction->setShortcuts(QKeySequence::ZoomOut);
    toolbar->addAction(zoomOutAction);
    setToolbarButtonRole(toolbar, zoomOutAction, QStringLiteral("zoom"));
    registerShortcutAction(zoomOutAction);

    m_zoomSpinBox = new QSpinBox(this);
    m_zoomSpinBox->setRange(BoardModel::MinZoomPercent, BoardModel::MaxZoomPercent);
    m_zoomSpinBox->setSuffix(QStringLiteral("%"));
    m_zoomSpinBox->setValue(m_model.settings().zoomPercent());
    m_zoomSpinBox->setKeyboardTracking(false);
    connect(m_zoomSpinBox, &QSpinBox::editingFinished, this, [this]() { applyZoomSpinBoxValue(); });
    toolbar->addWidget(createSteppedSpinBox(m_zoomSpinBox, 52));

    QAction *zoomInAction = addActionButton(QStringLiteral("zoom-in"), tr("Zoom In"), [this]() { performZoomBy(10); });
    zoomInAction->setShortcuts(QKeySequence::ZoomIn);
    toolbar->addAction(zoomInAction);
    setToolbarButtonRole(toolbar, zoomInAction, QStringLiteral("zoom"));
    registerShortcutAction(zoomInAction);
}

void MainWindow::createPageControls(PersistentToolBar *toolbar)
{
    m_pageLabel = new QLabel(this);
    m_pageLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_pageLabel->setContentsMargins(0, 0, 0, 0);
    m_pageLabel->setIndent(0);
    m_pageLabel->setMargin(0);
    m_pageLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    toolbar->addWidget(m_pageLabel);

    QAction *previousPageAction = addActionButton(QStringLiteral("page-prev"), tr("Previous Page"), [this]() {
        navigatePage(QStringLiteral("previous"));
    });
    previousPageAction->setShortcut(QKeySequence(QStringLiteral("Alt+Left")));
    toolbar->addAction(previousPageAction);
    setToolbarButtonRole(toolbar, previousPageAction, QStringLiteral("page"));
    registerShortcutAction(previousPageAction);

    QAction *nextPageAction = addActionButton(QStringLiteral("page-next"), tr("Next Page"), [this]() {
        navigatePage(QStringLiteral("next"));
    });
    nextPageAction->setShortcut(QKeySequence(QStringLiteral("Alt+Right")));
    toolbar->addAction(nextPageAction);
    setToolbarButtonRole(toolbar, nextPageAction, QStringLiteral("page"));
    registerShortcutAction(nextPageAction);

    m_pageAddAction = addActionButton(QStringLiteral("page-add"), tr("Add Page"), [this]() { performAddPage(); });
    m_pageAddAction->setShortcuts({
        QKeySequence(QStringLiteral("Ctrl+Shift+N")),
        QKeySequence(QStringLiteral("Meta+Shift+N")),
    });
    toolbar->addAction(m_pageAddAction);
    setToolbarButtonRole(toolbar, m_pageAddAction, QStringLiteral("page"));
    registerShortcutAction(m_pageAddAction);
    m_pageDeleteAction = addActionButton(QStringLiteral("page-delete"), tr("Delete Page"), [this]() {
        if (!m_model.canDeletePage())
            return;
        const QMessageBox::StandardButton answer = QMessageBox::question(this, tr("Delete Page"), tr("Delete this page?"));
        if (answer == QMessageBox::Yes) {
            rememberCurrentPageScrollPosition();
            m_model.deleteCurrentPage();
            m_canvas->refreshSize();
            restoreCurrentPageScrollPosition();
            updateUiState();
        }
    });
    m_pageDeleteAction->setShortcuts({
        QKeySequence(QStringLiteral("Ctrl+Shift+Backspace")),
        QKeySequence(QStringLiteral("Meta+Shift+Backspace")),
    });
    toolbar->addAction(m_pageDeleteAction);
    setToolbarButtonRole(toolbar, m_pageDeleteAction, QStringLiteral("danger"));
    registerShortcutAction(m_pageDeleteAction);

    m_pageLockAction = addActionButton(QStringLiteral("page-unlock"), tr("Lock Page"), [this]() {
        if (m_model.setCurrentPageLocked(!m_model.currentPageLocked())) {
            m_model.selectTool(Tool::Select);
            m_canvas->refreshSize();
            updateUiState();
        }
    });
    m_pageLockAction->setCheckable(true);
    toolbar->addAction(m_pageLockAction);
    setToolbarButtonRole(toolbar, m_pageLockAction, QStringLiteral("page"));
}

void MainWindow::setToolbarButtonRole(PersistentToolBar *toolbar, QAction *action, const QString &role)
{
    if (!toolbar || !action)
        return;
    QWidget *widget = toolbar->widgetForAction(action);
    if (!widget)
        return;
    widget->setProperty("toolbarRole", role);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

void MainWindow::updateSteppedSpinBoxButtons(QSpinBox *spinBox)
{
    if (!spinBox || !spinBox->parentWidget())
        return;
    const bool enabled = spinBox->isEnabled();
    const auto buttons = spinBox->parentWidget()->findChildren<QToolButton *>();
    for (QToolButton *button : buttons)
        button->setEnabled(enabled);
}

void MainWindow::showShortcutHelp()
{
    const QString shortcuts = tr(
        "<table>"
        "<tr><td><b>Select</b></td><td>Ctrl+1</td></tr>"
        "<tr><td><b>Pen</b></td><td>Ctrl+2</td></tr>"
        "<tr><td><b>Eraser</b></td><td>Ctrl+3</td></tr>"
        "<tr><td><b>Line</b></td><td>Ctrl+4</td></tr>"
        "<tr><td><b>Rectangle</b></td><td>Ctrl+5</td></tr>"
        "<tr><td><b>Circle</b></td><td>Ctrl+6</td></tr>"
        "<tr><td><b>Text</b></td><td>Ctrl+7</td></tr>"
        "<tr><td><b>Polyline</b></td><td>Ctrl+8</td></tr>"
        "<tr><td><b>Bezier</b></td><td>Ctrl+9</td></tr>"
        "<tr><td><b>Arc</b></td><td>Ctrl+0</td></tr>"
        "<tr><td><b>Delete selected objects</b></td><td>Delete / Backspace</td></tr>"
        "<tr><td><b>Move selected objects by 1 px</b></td><td>Arrow keys</td></tr>"
        "<tr><td><b>Group / Ungroup</b></td><td>Ctrl+G / Ctrl+Shift+G</td></tr>"
        "<tr><td><b>Move selected objects backward/forward one step</b></td><td>Ctrl+[ / Ctrl+]</td></tr>"
        "<tr><td><b>Move selected objects to back/front</b></td><td>Ctrl+Shift+[ / Ctrl+Shift+]</td></tr>"
        "<tr><td><b>Undo</b></td><td>Ctrl+Z</td></tr>"
        "<tr><td><b>Redo</b></td><td>Ctrl+Y / Ctrl+Shift+Z</td></tr>"
        "<tr><td><b>Zoom in/out</b></td><td>Standard Zoom In / Zoom Out</td></tr>"
        "<tr><td><b>Previous / Next page</b></td><td>Alt+Left / Alt+Right</td></tr>"
        "<tr><td><b>Add page</b></td><td>Ctrl+Shift+N</td></tr>"
        "<tr><td><b>Delete page</b></td><td>Ctrl+Shift+Backspace</td></tr>"
        "<tr><td><b>Start extending a polyline</b></td><td>Ctrl+endpoint click (Control+click on macOS)</td></tr>"
        "<tr><td><b>Close polyline/bezier</b></td><td>Ctrl+Enter / Ctrl+Shift+C</td></tr>"
        "<tr><td><b>Open polyline/bezier</b></td><td>Ctrl+Shift+O</td></tr>"
        "<tr><td><b>Open a closed polyline at a vertex</b></td><td>Ctrl+Shift+vertex click</td></tr>"
        "<tr><td><b>Confirm text</b></td><td>Ctrl+Enter</td></tr>"
        "<tr><td><b>Cancel text editing</b></td><td>Esc</td></tr>"
        "</table>");
    QMessageBox::information(this, tr("Shortcuts"), shortcuts);
}

void MainWindow::applyZoomSpinBoxValue()
{
    if (!m_zoomSpinBox)
        return;
    const int previousZoom = m_model.settings().zoomPercent();
    const int appliedZoom = m_model.setZoomPercent(m_zoomSpinBox->value());
    if (m_zoomSpinBox->value() != appliedZoom)
        m_zoomSpinBox->setValue(appliedZoom);
    if (appliedZoom != previousZoom)
        m_canvas->refreshSize();
    updateUiState();
}

void MainWindow::applyArrowHeadSettings()
{
    if (!m_startArrowComboBox || !m_endArrowComboBox)
        return;
    const auto startArrow = static_cast<ArrowHead>(m_startArrowComboBox->currentData().toInt());
    const auto endArrow = static_cast<ArrowHead>(m_endArrowComboBox->currentData().toInt());
    if (m_model.hasSelection()) {
        if (m_model.updateSelectedArrowHeads(startArrow, endArrow)) {
            m_canvas->update();
            updateUiState();
        }
        return;
    }
    m_model.setStartArrowHead(startArrow);
    m_model.setEndArrowHead(endArrow);
}

void MainWindow::applyStrokeStyleSettings()
{
    if (!m_strokeStyleComboBox)
        return;
    const auto strokeStyle = static_cast<StrokeStyle>(m_strokeStyleComboBox->currentData().toInt());
    if (m_model.hasSelection() && m_model.selectedElementType() != ElementType::Text
        && m_model.selectedElementType() != ElementType::Image) {
        if (m_model.updateSelectedStrokeStyle(strokeStyle)) {
            m_canvas->update();
            updateUiState();
        }
        return;
    }
    m_model.setStrokeStyle(strokeStyle);
}

void MainWindow::rememberCurrentPageScrollPosition()
{
    if (!m_scrollArea)
        return;
    m_model.setCurrentPageScrollPosition(QPoint(m_scrollArea->horizontalScrollBar()->value(),
                                                m_scrollArea->verticalScrollBar()->value()));
}

void MainWindow::restoreCurrentPageScrollPosition()
{
    if (!m_scrollArea)
        return;
    const QPoint scrollPosition = m_model.currentPageScrollPosition();
    m_scrollArea->horizontalScrollBar()->setValue(scrollPosition.x());
    m_scrollArea->verticalScrollBar()->setValue(scrollPosition.y());
}

void MainWindow::performUndo()
{
    if (m_model.undo()) {
        m_canvas->refreshSize();
        updateUiState();
    }
}

void MainWindow::performRedo()
{
    if (m_model.redo()) {
        m_canvas->refreshSize();
        updateUiState();
    }
}

void MainWindow::performDeleteSelected()
{
    if (m_model.deleteSelectedElement()) {
        m_canvas->update();
        updateUiState();
    }
}

void MainWindow::performCopy()
{
    m_clipboardElements = m_model.copySelectedElements();
    updateUiState();
}

void MainWindow::performPaste()
{
    if (m_model.pasteElements(m_clipboardElements)) {
        m_canvas->update();
        updateUiState();
        return;
    }
    if (performClipboardImagePaste()) {
        m_canvas->update();
        updateUiState();
    }
}

bool MainWindow::performClipboardImagePaste()
{
    if (m_model.currentPageLocked())
        return false;
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData || !mimeData->hasImage())
        return false;

    QImage image = imageFromMimeData(mimeData);
    if (image.isNull())
        return false;

    DrawingElement element;
    QString errorMessage;
    if (!ImageImporter::importFromImage(image, QSize(BoardModel::CanvasWidth, BoardModel::CanvasHeight), visibleCanvasCenter(), &element, &errorMessage)) {
        QMessageBox::critical(
            this,
            tr("Could Not Import Image"),
            tr("The image could not be imported.\n%1").arg(errorMessage));
        return false;
    }

    m_model.addImage(element.rect(), element.imageData(), element.imageMimeType());
    return true;
}

bool MainWindow::hasClipboardImage() const
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    return mimeData && mimeData->hasImage();
}

QPointF MainWindow::visibleCanvasCenter() const
{
    QPointF center(BoardModel::CanvasWidth / 2.0, BoardModel::CanvasHeight / 2.0);
    if (m_canvas && m_scrollArea && m_scrollArea->viewport()) {
        const QPoint widgetCenter = m_canvas->mapFrom(m_scrollArea->viewport(), m_scrollArea->viewport()->rect().center());
        const qreal scale = m_model.settings().zoomPercent() / 100.0;
        if (scale > 0.0) {
            center = QPointF(widgetCenter) / scale;
            center.setX(qBound(0.0, center.x(), static_cast<qreal>(BoardModel::CanvasWidth)));
            center.setY(qBound(0.0, center.y(), static_cast<qreal>(BoardModel::CanvasHeight)));
        }
    }
    return center;
}

void MainWindow::performResetStyle()
{
    if (m_model.hasSelection()) {
        if (m_model.resetSelectedStyle()) {
            m_canvas->update();
            updateUiState();
        }
        return;
    }
    m_model.resetDefaultStyle();
    updateUiState();
}

void MainWindow::performZoomBy(int deltaPercent)
{
    const int previousZoom = m_model.settings().zoomPercent();
    const int appliedZoom = m_model.setZoomPercent(previousZoom + deltaPercent);
    if (appliedZoom != previousZoom)
        m_canvas->refreshSize();
    updateUiState();
}

void MainWindow::performAddPage()
{
    rememberCurrentPageScrollPosition();
    if (m_model.addPage()) {
        m_canvas->refreshSize();
        restoreCurrentPageScrollPosition();
        updateUiState();
    }
}

void MainWindow::performDrawingImport()
{
    if (m_model.currentPageLocked()) return;
    const QString filePath = QFileDialog::getOpenFileName(
        this, tr("Import Drawing Data"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("Whiteboard drawing data (*.whiteboard *.json)"));
    if (filePath.isEmpty()) return;
    QVector<Page> pages;
    QString error;
    if (!BoardDocumentExchange::read(filePath, &pages, &error)) {
        QMessageBox::critical(this, tr("Could Not Import Drawing Data"), tr("The drawing data could not be imported.\n%1").arg(error));
        return;
    }
    if (pages.size() > BoardModel::MaxPages - m_model.document().pages().size()) {
        QMessageBox::warning(this, tr("Could Not Import Drawing Data"),
                             tr("Importing %1 pages would exceed the limit of %2 pages.").arg(pages.size()).arg(BoardModel::MaxPages));
        return;
    }
    m_canvas->commitPendingTextEdit();
    rememberCurrentPageScrollPosition();
    if (!m_model.importPages(pages, &error)) {
        QMessageBox::critical(this, tr("Could Not Import Drawing Data"), tr("The drawing data could not be imported.\n%1").arg(error));
        return;
    }
    m_canvas->refreshSize();
    updateUiState();
    saveDocument();
    QMessageBox::information(this, tr("Import Drawing Data"),
                             tr("Imported %1 pages after the current page. Undo removes this import.").arg(pages.size()));
}

void MainWindow::performDrawingExport()
{
    bool accepted = false;
    const QStringList scopes{tr("All pages"), tr("Current page")};
    const QString choice = QInputDialog::getItem(this, tr("Export Drawing Data"), tr("Export scope:"), scopes, 0, false, &accepted);
    if (!accepted) return;
    QFileDialog dialog(this, tr("Export Drawing Data"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Whiteboard drawing data (*.whiteboard)"));
    dialog.setDefaultSuffix(QStringLiteral("whiteboard"));
    dialog.selectFile(QStringLiteral("drawing.whiteboard"));
    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) return;
    const QString filePath = dialog.selectedFiles().first();
    // Do not overwrite the application's autosave with the portable format.
    if (QFileInfo(filePath).suffix().compare(QStringLiteral("whiteboard"), Qt::CaseInsensitive) != 0) {
        QMessageBox::warning(this, tr("Could Not Export Drawing Data"), tr("Use the .whiteboard file extension."));
        return;
    }
    m_canvas->commitPendingTextEdit();
    rememberCurrentPageScrollPosition();
    updateUiState();
    QString error;
    const auto scope = choice == scopes.first() ? BoardDocumentExchange::Scope::AllPages : BoardDocumentExchange::Scope::CurrentPage;
    if (!BoardDocumentExchange::write(filePath, m_model.document(), scope, &error))
        QMessageBox::critical(this, tr("Could Not Export Drawing Data"), tr("The drawing data could not be exported.\n%1").arg(error));
}

void MainWindow::performImageImport()
{
    m_canvas->commitPendingTextEdit();
    updateUiState();
    if (m_model.currentPageLocked())
        return;

    QString directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (directory.isEmpty())
        directory = QDir::homePath();

    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Import Image"),
        directory,
        tr("Images (*.png *.jpg *.jpeg *.bmp *.svg)"));
    if (filePath.isEmpty())
        return;

    DrawingElement element;
    QString errorMessage;
    if (!ImageImporter::importFromFile(filePath, QSize(BoardModel::CanvasWidth, BoardModel::CanvasHeight), visibleCanvasCenter(), &element, &errorMessage)) {
        QMessageBox::critical(
            this,
            tr("Could Not Import Image"),
            tr("The image could not be imported.\n%1").arg(errorMessage));
        return;
    }

    m_model.addImage(element.rect(), element.imageData(), element.imageMimeType());
    m_canvas->refreshSize();
    updateUiState();
    saveDocument();
}

void MainWindow::performImageExport()
{
    m_canvas->commitPendingTextEdit();
    updateUiState();
    const Page page = m_model.currentPage();
    if (!ImageExporter::hasExportableElements(page))
        return;

    ImageExportDialog settingsDialog(this);
    if (settingsDialog.exec() != QDialog::Accepted)
        return;

    ImageExporter::Options options;
    options.format = settingsDialog.format();
    options.background = settingsDialog.background();

    const QString pngFilter = tr("PNG Images (*.png)");
    const QString svgFilter = tr("SVG Images (*.svg)");
    QString selectedFilter = options.format == ImageExporter::Format::Png ? pngFilter : svgFilter;
    QString directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (directory.isEmpty())
        directory = QDir::homePath();
    const QString baseName = tr("whiteboard-page-%1").arg(m_model.document().currentPageIndex() + 1);
    QString filePath;

    while (filePath.isEmpty()) {
        QFileDialog fileDialog(this, tr("Save Image"), directory);
        fileDialog.setAcceptMode(QFileDialog::AcceptSave);
        fileDialog.setFileMode(QFileDialog::AnyFile);
        fileDialog.setNameFilters({pngFilter, svgFilter});
        fileDialog.selectNameFilter(selectedFilter);
        fileDialog.selectFile(baseName);
        fileDialog.setDefaultSuffix(ImageExporter::extension(
            selectedFilter == svgFilter ? ImageExporter::Format::Svg : ImageExporter::Format::Png));
        connect(&fileDialog, &QFileDialog::filterSelected, &fileDialog,
                [&fileDialog, svgFilter](const QString &filter) {
                    fileDialog.setDefaultSuffix(ImageExporter::extension(
                        filter == svgFilter ? ImageExporter::Format::Svg : ImageExporter::Format::Png));
                });
        if (fileDialog.exec() != QDialog::Accepted || fileDialog.selectedFiles().isEmpty())
            return;
        selectedFilter = fileDialog.selectedNameFilter();
        filePath = fileDialog.selectedFiles().first();

        const ImageExporter::Format filterFormat = selectedFilter == svgFilter
            ? ImageExporter::Format::Svg : ImageExporter::Format::Png;
        filePath = ImageExporter::withDefaultExtension(filePath, filterFormat);
        if (!ImageExporter::formatForFilePath(filePath, filterFormat, &options.format)) {
            const QMessageBox::StandardButton answer = QMessageBox::warning(
                this,
                tr("Unsupported Image Format"),
                tr("Use a .png or .svg file extension."),
                QMessageBox::Retry | QMessageBox::Cancel,
                QMessageBox::Retry);
            if (answer != QMessageBox::Retry)
                return;
            directory = QFileInfo(filePath).absolutePath();
            filePath.clear();
        }
    }

    const QVector<int> selectedIndexes = m_model.hasSelection()
        ? m_model.selectedElementIndexes() : QVector<int>();
    while (true) {
        QString errorMessage;
        if (ImageExporter::save(page, selectedIndexes, options, filePath, &errorMessage))
            return;
        const QMessageBox::StandardButton answer = QMessageBox::critical(
            this,
            tr("Could Not Save Image"),
            tr("The image could not be saved.\n%1").arg(errorMessage),
            QMessageBox::Retry | QMessageBox::Cancel,
            QMessageBox::Retry);
        if (answer != QMessageBox::Retry)
            return;
    }
}

void MainWindow::setFloatingActionsExpanded(bool expanded)
{
    const bool wasExpanded = m_floatingActionsExpanded;
    const QVector<QToolButton *> actionButtons = {
        m_floatingSelectButton,
        m_floatingResetButton,
        m_floatingUndoButton,
        m_floatingRedoButton,
        m_floatingCopyButton,
        m_floatingPasteButton,
    };
    if (m_floatingMainButton) {
        m_floatingMainButton->setToolTip(expanded ? tr("Close Quick Actions") : tr("Quick Actions"));
        m_floatingMainButton->setAccessibleName(m_floatingMainButton->toolTip());
    }
    if (!m_floatingActionContainer)
        return;

    if (m_floatingActionsAnimation) {
        m_floatingActionsAnimation->stop();
        QObject::disconnect(m_floatingActionsAnimation, &QPropertyAnimation::finished, this, nullptr);
    }
    if (m_floatingActionsOpacityAnimation)
        m_floatingActionsOpacityAnimation->stop();

    constexpr int slideOffset = 20;
    if (expanded) {
        m_floatingActionsExpanded = true;
        for (QToolButton *button : actionButtons) {
            if (button)
                button->setVisible(true);
        }
        m_floatingActionContainer->show();
        updateFloatingActionButtonPositions();

        const QRect endGeometry = m_floatingActionContainer->geometry();
        const bool containerAboveMain = m_floatingMainButton
            && endGeometry.center().y() < m_floatingMainButton->geometry().center().y();
        const QRect startGeometry = endGeometry.translated(0, containerAboveMain ? slideOffset : -slideOffset);
        m_floatingActionContainer->setGeometry(startGeometry);
        m_floatingActionContainer->raise();
        m_floatingMainButton->raise();
        if (m_floatingActionsOpacityEffect)
            m_floatingActionsOpacityEffect->setOpacity(0.0);
        if (m_floatingActionsAnimation) {
            m_floatingActionsAnimation->setStartValue(startGeometry);
            m_floatingActionsAnimation->setEndValue(endGeometry);
            m_floatingActionsAnimation->start();
        } else {
            m_floatingActionContainer->setGeometry(endGeometry);
        }
        if (m_floatingActionsOpacityAnimation) {
            m_floatingActionsOpacityAnimation->setStartValue(0.0);
            m_floatingActionsOpacityAnimation->setEndValue(1.0);
            m_floatingActionsOpacityAnimation->start();
        } else if (m_floatingActionsOpacityEffect) {
            m_floatingActionsOpacityEffect->setOpacity(1.0);
        }
        return;
    }

    if (!wasExpanded) {
        m_floatingActionsExpanded = false;
        for (QToolButton *button : actionButtons) {
            if (button)
                button->setVisible(false);
        }
        m_floatingActionContainer->hide();
        if (m_floatingActionsOpacityEffect)
            m_floatingActionsOpacityEffect->setOpacity(0.0);
        updateFloatingActionButtonPositions();
        return;
    }

    const QRect startGeometry = m_floatingActionContainer->geometry();
    const bool containerAboveMain = m_floatingMainButton
        && startGeometry.center().y() < m_floatingMainButton->geometry().center().y();
    const QRect endGeometry = startGeometry.translated(0, containerAboveMain ? slideOffset : -slideOffset);
    m_floatingActionsExpanded = false;
    for (QToolButton *button : actionButtons) {
        if (button)
            button->setVisible(true);
    }
    m_floatingActionContainer->show();
    if (m_floatingActionsOpacityEffect)
        m_floatingActionsOpacityEffect->setOpacity(1.0);
    if (m_floatingActionsAnimation) {
        m_floatingActionsAnimation->setStartValue(startGeometry);
        m_floatingActionsAnimation->setEndValue(endGeometry);
        connect(m_floatingActionsAnimation, &QPropertyAnimation::finished, this, [this, actionButtons]() {
            for (QToolButton *button : actionButtons) {
                if (button)
                    button->setVisible(false);
            }
            if (m_floatingActionContainer)
                m_floatingActionContainer->hide();
            if (m_floatingActionsOpacityEffect)
                m_floatingActionsOpacityEffect->setOpacity(0.0);
            updateFloatingActionButtonPositions();
        });
        m_floatingActionsAnimation->start();
    } else {
        for (QToolButton *button : actionButtons) {
            if (button)
                button->setVisible(false);
        }
        m_floatingActionContainer->hide();
        if (m_floatingActionsOpacityEffect)
            m_floatingActionsOpacityEffect->setOpacity(0.0);
        updateFloatingActionButtonPositions();
    }
    if (m_floatingActionsOpacityAnimation) {
        m_floatingActionsOpacityAnimation->setStartValue(1.0);
        m_floatingActionsOpacityAnimation->setEndValue(0.0);
        m_floatingActionsOpacityAnimation->start();
    }
}

void MainWindow::updateFloatingActionDockGeometry()
{
    if (!m_floatingActionPanel || !m_floatingActionPanel->parentWidget())
        return;

    updateFloatingActionButtonPositions();
}

void MainWindow::updateFloatingActionButtonPositions()
{
    if (!m_floatingActionPanel || !m_floatingMainButton || !m_floatingActionPanel->parentWidget())
        return;

    constexpr int panelWidth = 64;
    constexpr int rightMargin = 24;
    constexpr int verticalMargin = 24;
    constexpr int buttonSize = 40;
    constexpr int panelSpacing = 8;
    constexpr int actionButtonCount = 6;
    constexpr int actionWidth = 40;
    const int expandedHeight = (buttonSize * actionButtonCount) + (panelSpacing * (actionButtonCount - 1));
    QWidget *parent = m_floatingActionPanel->parentWidget();
    const int parentHeight = parent->height();
    const int mainHeight = m_floatingMainButton->height();
    if (m_floatingActionDockY < 0)
        m_floatingActionDockY = parentHeight - mainHeight - verticalMargin;
    m_floatingActionDockY = qBound(verticalMargin,
                                   m_floatingActionDockY,
                                   qMax(verticalMargin, parentHeight - mainHeight - verticalMargin));

    const int panelX = qMax(0, parent->width() - panelWidth - rightMargin);
    const int mainLeft = (panelWidth - m_floatingMainButton->width()) / 2;
    const int actionLeft = (panelWidth - actionWidth) / 2;
    int panelTop = m_floatingActionDockY;
    int panelBottom = m_floatingActionDockY + mainHeight;
    int actionTop = 0;
    if (m_floatingActionsExpanded) {
        const bool expandUp = (m_floatingActionDockY + (mainHeight / 2)) >= (parentHeight / 2);
        if (m_floatingActionContainer) {
            if (auto *layout = qobject_cast<QVBoxLayout *>(m_floatingActionContainer->layout())) {
                QVector<QToolButton *> displayedButtons = {
                    m_floatingSelectButton,
                    m_floatingResetButton,
                    m_floatingUndoButton,
                    m_floatingRedoButton,
                    m_floatingCopyButton,
                    m_floatingPasteButton,
                };
                if (expandUp)
                    std::reverse(displayedButtons.begin(), displayedButtons.end());
                for (QToolButton *button : displayedButtons) {
                    if (button)
                        layout->addWidget(button);
                }
                if (!displayedButtons.isEmpty() && displayedButtons.first())
                    QWidget::setTabOrder(m_floatingMainButton, displayedButtons.first());
                for (qsizetype index = 1; index < displayedButtons.size(); ++index) {
                    if (displayedButtons[index - 1] && displayedButtons[index])
                        QWidget::setTabOrder(displayedButtons[index - 1], displayedButtons[index]);
                }
            }
        }
        const int preferredActionTop = expandUp
            ? m_floatingActionDockY - panelSpacing - expandedHeight
            : m_floatingActionDockY + mainHeight + panelSpacing;
        actionTop = qBound(8,
                           preferredActionTop,
                           qMax(8, parentHeight - expandedHeight - 8));
        panelTop = qMin(panelTop, actionTop);
        panelBottom = qMax(panelBottom, actionTop + expandedHeight);
    } else if (m_floatingActionContainer) {
        m_floatingActionContainer->hide();
    }
    m_floatingActionPanel->setGeometry(panelX, panelTop, panelWidth, panelBottom - panelTop);
    m_floatingMainButton->move(mainLeft, m_floatingActionDockY - panelTop);
    if (m_floatingActionsExpanded && m_floatingActionContainer) {
        m_floatingActionContainer->setGeometry(actionLeft, actionTop - panelTop, actionWidth, expandedHeight);
        m_floatingActionContainer->show();
        m_floatingActionContainer->raise();
    }
    m_floatingMainButton->raise();
    m_floatingActionPanel->raise();
    m_floatingActionPanel->update();
}

void MainWindow::updateFloatingActionPanelMask()
{
    if (!m_floatingActionPanel)
        return;
    m_floatingActionPanel->clearMask();
}

void MainWindow::loadDocument()
{
    m_model.loadFromFile(saveFilePath());
}

void MainWindow::saveDocument()
{
    m_model.saveToFile(saveFilePath());
}

void MainWindow::updateUiState()
{
    const BoardSettings settings = m_model.settings();
    const bool pageLocked = m_model.currentPageLocked();
    const bool hasSelection = m_model.hasSelection();
    const ElementType selectedType = hasSelection ? m_model.selectedElementType() : ElementType::Freehand;
    const bool selectedClosed = hasSelection && m_model.selectedElementClosed();
    const bool textSelected = hasSelection && selectedType == ElementType::Text;
    const bool imageSelected = hasSelection && selectedType == ElementType::Image;
    const bool canEditPage = !pageLocked;
    const bool pasteAvailable = !m_clipboardElements.isEmpty() || hasClipboardImage();
    const bool fillEnabled = canEditPage && !imageSelected && (!hasSelection || supportsFillControl(selectedType, selectedClosed));
    const bool arrowEnabled = canEditPage && !imageSelected && (!hasSelection || supportsArrowControls(selectedType, selectedClosed));
    const Tool displayedTool = pageLocked ? Tool::Select : settings.selectedTool();
    for (QAction *action : m_toolActions) {
        const Tool tool = static_cast<Tool>(action->data().toInt());
        action->setChecked(tool == displayedTool);
        action->setEnabled(!pageLocked || tool == Tool::Select);
    }
    if (m_zoomSpinBox && m_zoomSpinBox->value() != settings.zoomPercent())
        m_zoomSpinBox->setValue(settings.zoomPercent());
    updateSteppedSpinBoxButtons(m_zoomSpinBox);
    if (m_strokeSpinBox) {
        const int strokeWidth = (hasSelection && !textSelected && !imageSelected)
            ? m_model.selectedElementStrokeWidth()
            : settings.strokeWidth();
        const QSignalBlocker blocker(m_strokeSpinBox);
        if (m_strokeSpinBox->value() != strokeWidth)
            m_strokeSpinBox->setValue(strokeWidth);
        m_strokeSpinBox->setEnabled(canEditPage && !textSelected && !imageSelected);
        updateSteppedSpinBoxButtons(m_strokeSpinBox);
    }
    if (m_cornerRadiusSpinBox) {
        const int cornerRadius = (hasSelection && selectedType == ElementType::RoundedRectangle)
            ? m_model.selectedElementCornerRadius()
            : settings.cornerRadius();
        const QSignalBlocker blocker(m_cornerRadiusSpinBox);
        if (m_cornerRadiusSpinBox->value() != cornerRadius)
            m_cornerRadiusSpinBox->setValue(cornerRadius);
        m_cornerRadiusSpinBox->setEnabled(canEditPage && (!hasSelection || selectedType == ElementType::RoundedRectangle));
        updateSteppedSpinBoxButtons(m_cornerRadiusSpinBox);
    }
    if (m_strokeStyleComboBox) {
        const StrokeStyle strokeStyle = (hasSelection && !textSelected && !imageSelected)
            ? m_model.selectedElementStrokeStyle()
            : settings.strokeStyle();
        const QSignalBlocker blocker(m_strokeStyleComboBox);
        m_strokeStyleComboBox->setCurrentIndex(m_strokeStyleComboBox->findData(static_cast<int>(strokeStyle)));
        m_strokeStyleComboBox->setEnabled(canEditPage && !textSelected && !imageSelected);
    }
    if (m_startArrowComboBox && m_endArrowComboBox) {
        const ArrowHead startArrow = hasSelection ? m_model.selectedElementStartArrowHead() : settings.startArrowHead();
        const ArrowHead endArrow = hasSelection ? m_model.selectedElementEndArrowHead() : settings.endArrowHead();
        const QSignalBlocker startBlocker(m_startArrowComboBox);
        const QSignalBlocker endBlocker(m_endArrowComboBox);
        m_startArrowComboBox->setCurrentIndex(m_startArrowComboBox->findData(static_cast<int>(startArrow)));
        m_endArrowComboBox->setCurrentIndex(m_endArrowComboBox->findData(static_cast<int>(endArrow)));
        m_startArrowComboBox->setEnabled(arrowEnabled);
        m_endArrowComboBox->setEnabled(arrowEnabled);
    }
    if (m_colorAction)
        m_colorAction->setEnabled(canEditPage && !imageSelected);
    if (m_fillAction)
        m_fillAction->setEnabled(fillEnabled);
    if (m_fontAction)
        m_fontAction->setEnabled(canEditPage && (!hasSelection || textSelected) && !imageSelected);
    if (m_resetStyleAction)
        m_resetStyleAction->setEnabled(canEditPage && (!hasSelection || !imageSelected || m_model.selectedElementIndexes().size() > 1));
    if (m_scaleAction)
        m_scaleAction->setEnabled(canEditPage && hasSelection && (!textSelected || m_model.selectedElementIndexes().size() > 1));
    if (m_undoAction)
        m_undoAction->setEnabled(m_model.canUndo());
    if (m_redoAction)
        m_redoAction->setEnabled(m_model.canRedo());
    if (m_copyAction)
        m_copyAction->setEnabled(hasSelection);
    if (m_pasteAction)
        m_pasteAction->setEnabled(canEditPage && pasteAvailable);
    if (m_groupAction)
        m_groupAction->setEnabled(canEditPage && m_model.selectedElementIndexes().size() >= 2);
    if (m_ungroupAction)
        m_ungroupAction->setEnabled(canEditPage && hasSelection && m_model.selectedSelectionHasGroup());
    if (m_sendBackwardAction)
        m_sendBackwardAction->setEnabled(canEditPage && hasSelection);
    if (m_bringForwardAction)
        m_bringForwardAction->setEnabled(canEditPage && hasSelection);
    if (m_sendToBackAction)
        m_sendToBackAction->setEnabled(canEditPage && hasSelection);
    if (m_bringToFrontAction)
        m_bringToFrontAction->setEnabled(canEditPage && hasSelection);
    if (m_imageExportAction) {
        const bool hasElements = !m_model.currentPage().elements().isEmpty();
        m_imageExportAction->setEnabled(hasElements || (m_canvas && m_canvas->hasPendingTextEdit()));
    }
    if (m_imageImportAction)
        m_imageImportAction->setEnabled(canEditPage);
    if (m_drawingImportAction)
        m_drawingImportAction->setEnabled(canEditPage && m_model.document().pages().size() < BoardModel::MaxPages);
    if (m_pageAddAction)
        m_pageAddAction->setEnabled(m_model.canAddPage());
    if (m_pageDeleteAction)
        m_pageDeleteAction->setEnabled(m_model.canDeletePage());
    if (m_pageLockAction) {
        m_pageLockAction->setChecked(pageLocked);
        m_pageLockAction->setText(pageLocked ? tr("Unlock Page") : tr("Lock Page"));
        m_pageLockAction->setToolTip(pageLocked ? tr("Unlock Page") : tr("Lock Page"));
        m_pageLockAction->setIcon(QIcon(iconPath(pageLocked ? QStringLiteral("page-lock") : QStringLiteral("page-unlock"))));
    }
    if (m_floatingSelectButton)
        m_floatingSelectButton->setEnabled(true);
    if (m_floatingResetButton)
        m_floatingResetButton->setEnabled(canEditPage);
    if (m_floatingUndoButton)
        m_floatingUndoButton->setEnabled(m_model.canUndo());
    if (m_floatingRedoButton)
        m_floatingRedoButton->setEnabled(m_model.canRedo());
    if (m_floatingCopyButton)
        m_floatingCopyButton->setEnabled(hasSelection);
    if (m_floatingPasteButton)
        m_floatingPasteButton->setEnabled(canEditPage && pasteAvailable);
    if (m_pageLabel) {
        const QString pageText = (pageLocked ? tr("Locked ") : QString()) + tr("%1 / %2")
            .arg(m_model.document().currentPageIndex() + 1)
            .arg(m_model.document().pages().size());
        m_pageLabel->setText(pageText);
        if (m_persistentToolbar)
            m_persistentToolbar->refreshOverflow();
    }
}
