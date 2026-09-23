#pragma once

#include "BoardModel.h"
#include "FloatingActionButton.h"

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QPointF>
#include <QSpinBox>

#include <functional>

class CanvasWidget;
class FloatingActionChildButton;
class PersistentToolBar;
class QComboBox;
class QFrame;
class QGraphicsOpacityEffect;
class QJsonObject;
class QMenu;
class QPropertyAnimation;
class QScrollArea;
class QTimer;
class QToolBar;
class QToolButton;
class QWidget;
class WhiteboardMcpController;
class WhiteboardMcpHttpServer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool startMcpServer();
    bool startMcpHttpServer();
    QString mcpHttpErrorString() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    QAction *addToolAction(const QString &iconName, const QString &text, Tool tool);
    QAction *addActionButton(const QString &iconName, const QString &text, const std::function<void()> &handler);
    FloatingActionChildButton *addFloatingActionButton(QWidget *parent,
                                                       QWidget *labelParent,
                                                       const QString &iconName,
                                                       const QString &text,
                                                       const std::function<void()> &handler);
    QComboBox *createArrowComboBox(const QString &labelPrefix);
    QWidget *createSteppedSpinBox(QSpinBox *spinBox, int valueWidth);
    void registerShortcutAction(QAction *action);
    void registerDelayedToolHelp(QWidget *widget, const QString &text);
    void hideDelayedToolHelp();
    QString iconPath(const QString &iconName) const;
    QString saveFilePath() const;
    void createFileMenu();
    void createHelpMenu();
    void createToolbar();
    void createFloatingActions(QWidget *parent);
    void createToolButtons(PersistentToolBar *toolbar);
    void createEditActions(PersistentToolBar *toolbar);
    void createStyleControls(PersistentToolBar *toolbar);
    void createZoomControls(PersistentToolBar *toolbar);
    void createPageControls(PersistentToolBar *toolbar);
    void setToolbarButtonRole(PersistentToolBar *toolbar, QAction *action, const QString &role);
    void updateSteppedSpinBoxButtons(QSpinBox *spinBox);
    void showShortcutHelp();
    void applyZoomSpinBoxValue();
    void applyArrowHeadSettings();
    void applyStrokeStyleSettings();
    void rememberCurrentPageScrollPosition();
    void restoreCurrentPageScrollPosition();
    bool navigatePage(const QString &direction);
    void handleMcpChange();
    bool approveMcpRequest(const QString &toolName,
                           const QJsonObject &arguments,
                           QString *denyReason);
    void performUndo();
    void performRedo();
    void performDeleteSelected();
    void performCopy();
    void performPaste();
    void performResetStyle();
    void performZoomBy(int deltaPercent);
    void performAddPage();
    void performImageImport();
    void performDrawingImport();
    void performDrawingExport();
    bool performClipboardImagePaste();
    void performImageExport();
    bool hasClipboardImage() const;
    QPointF visibleCanvasCenter() const;
    void setFloatingActionsExpanded(bool expanded);
    void updateFloatingActionDockGeometry();
    void updateFloatingActionButtonPositions();
    void updateFloatingActionPanelMask();
    void loadDocument();
    void saveDocument();
    void updateUiState();

    BoardModel m_model;
    CanvasWidget *m_canvas = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QToolBar *m_toolbar = nullptr;
    PersistentToolBar *m_persistentToolbar = nullptr;
    QLabel *m_pageLabel = nullptr;
    QSpinBox *m_zoomSpinBox = nullptr;
    QSpinBox *m_strokeSpinBox = nullptr;
    QSpinBox *m_cornerRadiusSpinBox = nullptr;
    QComboBox *m_strokeStyleComboBox = nullptr;
    QComboBox *m_startArrowComboBox = nullptr;
    QComboBox *m_endArrowComboBox = nullptr;
    QTimer *m_toolHelpDelayTimer = nullptr;
    QTimer *m_toolHelpHideTimer = nullptr;
    QWidget *m_toolHelpWidget = nullptr;
    QMenu *m_editMenu = nullptr;
    QAction *m_imageImportAction = nullptr;
    QAction *m_drawingImportAction = nullptr;
    QAction *m_imageExportAction = nullptr;
    QAction *m_pageAddAction = nullptr;
    QAction *m_pageDeleteAction = nullptr;
    QAction *m_pageLockAction = nullptr;
    QAction *m_colorAction = nullptr;
    QAction *m_fillAction = nullptr;
    QAction *m_fontAction = nullptr;
    QAction *m_resetStyleAction = nullptr;
    QAction *m_scaleAction = nullptr;
    QAction *m_undoAction = nullptr;
    QAction *m_redoAction = nullptr;
    QAction *m_copyAction = nullptr;
    QAction *m_pasteAction = nullptr;
    QAction *m_groupAction = nullptr;
    QAction *m_ungroupAction = nullptr;
    QAction *m_sendBackwardAction = nullptr;
    QAction *m_bringForwardAction = nullptr;
    QAction *m_sendToBackAction = nullptr;
    QAction *m_bringToFrontAction = nullptr;
    FloatingActionButton *m_floatingMainButton = nullptr;
    QFrame *m_floatingActionPanel = nullptr;
    QWidget *m_floatingActionContainer = nullptr;
    QGraphicsOpacityEffect *m_floatingActionsOpacityEffect = nullptr;
    QPropertyAnimation *m_floatingActionsAnimation = nullptr;
    QPropertyAnimation *m_floatingActionsOpacityAnimation = nullptr;
    QToolButton *m_floatingSelectButton = nullptr;
    QToolButton *m_floatingResetButton = nullptr;
    QToolButton *m_floatingUndoButton = nullptr;
    QToolButton *m_floatingRedoButton = nullptr;
    QToolButton *m_floatingCopyButton = nullptr;
    QToolButton *m_floatingPasteButton = nullptr;
    WhiteboardMcpController *m_mcpController = nullptr;
    WhiteboardMcpHttpServer *m_mcpHttpServer = nullptr;
    bool m_floatingActionsExpanded = false;
    int m_floatingActionDockY = -1;
    QVector<QAction *> m_toolActions;
    QVector<DrawingElement> m_clipboardElements;
};
