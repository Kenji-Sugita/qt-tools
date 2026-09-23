#pragma once

#include "../src/objectpicker.h"
#include "../src/objecttreedialog.h"
#include "../src/objecttreewidget.h"
#include "../src/propertyeditor.h"

#include <algorithm>
#include <functional>
#include <memory>

#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QPointer>
#include <QtCore/QTime>
#include <QtCore/QUrl>
#include <QtCore/QVector>
#include <QtGui/QKeySequence>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QCalendarWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCommandLinkButton>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QDial>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsRectItem>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QKeySequenceEdit>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#if defined(HAVE_QT_OPENGLWIDGETS) || __has_include(<QOpenGLWidget>)
#  include <QOpenGLWidget>
#  define DEMO_HAS_OPENGL_WIDGET 1
#else
#  define DEMO_HAS_OPENGL_WIDGET 0
#endif

#if defined(HAVE_QT_QUICKWIDGETS) || __has_include(<QtQuickWidgets/QQuickWidget>)
#  include <QtQuickWidgets/QQuickWidget>
#  define DEMO_HAS_QUICK_WIDGET 1
#else
#  define DEMO_HAS_QUICK_WIDGET 0
#endif

#if defined(HAVE_QT_WEBENGINEWIDGETS) || __has_include(<QtWebEngineWidgets/QWebEngineView>)
#  include <QtWebEngineWidgets/QWebEngineView>
#  define DEMO_HAS_WEBENGINE_VIEW 1
#else
#  define DEMO_HAS_WEBENGINE_VIEW 0
#endif

namespace DemoWidgetSelector {

struct DemoWidgetSpecification {
    QString className;
    std::function<QWidget *(QWidget *parentWidget)> createWidget;
};

inline QWidget *createUnavailableModuleWidget(const QString &className, QWidget *parentWidget)
{
    auto *placeholderWidget = new QWidget(parentWidget);
    auto *placeholderLayout = new QVBoxLayout(placeholderWidget);

    auto *titleLabel = new QLabel(className, placeholderWidget);
    auto *messageLabel = new QLabel(
        QStringLiteral("This Qt module is not available in the current build environment."),
        placeholderWidget);
    messageLabel->setWordWrap(true);

    placeholderLayout->addWidget(titleLabel);
    placeholderLayout->addWidget(messageLabel);
    placeholderLayout->addStretch();
    placeholderWidget->setMinimumSize(320, 180);
    return placeholderWidget;
}

inline QWidget *createPushButtonWidget(QWidget *parentWidget)
{
    auto *pushButton = new QPushButton(QStringLiteral("Push Button"), parentWidget);
    return pushButton;
}

inline QWidget *createToolButtonWidget(QWidget *parentWidget)
{
    auto *toolButton = new QToolButton();
    toolButton->setText(QStringLiteral("Tool Button"));
    toolButton->setCheckable(true);
    toolButton->setChecked(true);
    toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    return toolButton;
}

inline QWidget *createRadioButtonWidget(QWidget *parentWidget)
{
    auto *radioButton = new QRadioButton(QStringLiteral("Radio Button"), parentWidget);
    radioButton->setChecked(true);
    radioButton->setMinimumSize(180, 28);
    return radioButton;
}

inline QWidget *createCheckBoxWidget(QWidget *parentWidget)
{
    auto *checkBox = new QCheckBox(QStringLiteral("Check Box"), parentWidget);
    checkBox->setChecked(true);
    checkBox->setMinimumSize(180, 28);
    return checkBox;
}

inline QWidget *createCommandLinkButtonWidget(QWidget *parentWidget)
{
    auto *commandLinkButton = new QCommandLinkButton(QStringLiteral("Command Link Button"),
                                                     QStringLiteral("Description text"),
                                                     parentWidget);
    commandLinkButton->setCheckable(true);
    commandLinkButton->setChecked(false);
    commandLinkButton->setMinimumSize(260, 64);
    return commandLinkButton;
}

inline QWidget *createDialogButtonBoxWidget(QWidget *parentWidget)
{
    auto *dialogButtonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, parentWidget);
    dialogButtonBox->setCenterButtons(false);
    dialogButtonBox->setOrientation(Qt::Horizontal);
	dialogButtonBox->setMinimumWidth(500);
    return dialogButtonBox;
}

inline QWidget *createListViewWidget(QWidget *parentWidget)
{
    auto *listView = new QListView(parentWidget);
    auto *model = new QStandardItemModel(listView);
    model->appendRow(new QStandardItem(QStringLiteral("Alpha")));
    model->appendRow(new QStandardItem(QStringLiteral("Beta")));
    model->appendRow(new QStandardItem(QStringLiteral("Gamma")));
    listView->setModel(model);
    listView->setMinimumSize(280, 180);
    return listView;
}

inline QWidget *createTableViewWidget(QWidget *parentWidget)
{
    auto *tableView = new QTableView(parentWidget);
    auto *model = new QStandardItemModel(3, 3, tableView);
    model->setHorizontalHeaderLabels(
        {QStringLiteral("Column 1"), QStringLiteral("Column 2"), QStringLiteral("Column 3")});
    for (int rowIndex = 0; rowIndex < model->rowCount(); ++rowIndex) {
        for (int columnIndex = 0; columnIndex < model->columnCount(); ++columnIndex) {
            model->setItem(rowIndex, columnIndex,
                           new QStandardItem(QStringLiteral("%1,%2")
                                                 .arg(rowIndex + 1)
                                                 .arg(columnIndex + 1)));
        }
    }
    tableView->setModel(model);
    tableView->setMinimumSize(320, 180);
    return tableView;
}

inline QWidget *createTreeViewWidget(QWidget *parentWidget)
{
    auto *treeView = new QTreeView(parentWidget);
    auto *model = new QStandardItemModel(treeView);
    model->setHorizontalHeaderLabels({QStringLiteral("Name"), QStringLiteral("Value")});

    auto *firstParentItem = new QStandardItem(QStringLiteral("Parent A"));
    firstParentItem->appendRow(
        {new QStandardItem(QStringLiteral("Child A1")), new QStandardItem(QStringLiteral("10"))});
    firstParentItem->appendRow(
        {new QStandardItem(QStringLiteral("Child A2")), new QStandardItem(QStringLiteral("20"))});

    auto *secondParentItem = new QStandardItem(QStringLiteral("Parent B"));
    secondParentItem->appendRow(
        {new QStandardItem(QStringLiteral("Child B1")), new QStandardItem(QStringLiteral("30"))});

    model->appendRow({firstParentItem, new QStandardItem(QStringLiteral("Root A"))});
    model->appendRow({secondParentItem, new QStandardItem(QStringLiteral("Root B"))});

    treeView->setModel(model);
    treeView->expandAll();
    treeView->setMinimumSize(320, 200);
    return treeView;
}

inline QWidget *createListWidgetWidget(QWidget *parentWidget)
{
    auto *listWidget = new QListWidget(parentWidget);
    listWidget->addItems({QStringLiteral("One"), QStringLiteral("Two"), QStringLiteral("Three")});
    listWidget->setCurrentRow(1);
    listWidget->setMinimumSize(240, 180);
    return listWidget;
}

inline QWidget *createTableWidgetWidget(QWidget *parentWidget)
{
    auto *tableWidget = new QTableWidget(3, 3, parentWidget);
    tableWidget->setHorizontalHeaderLabels(
        {QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")});
    for (int rowIndex = 0; rowIndex < tableWidget->rowCount(); ++rowIndex) {
        for (int columnIndex = 0; columnIndex < tableWidget->columnCount(); ++columnIndex) {
            tableWidget->setItem(rowIndex, columnIndex,
                                 new QTableWidgetItem(QStringLiteral("Item %1")
                                                          .arg(rowIndex * 3 + columnIndex + 1)));
        }
    }
    tableWidget->setMinimumSize(320, 180);
    return tableWidget;
}

inline QWidget *createTreeWidgetWidget(QWidget *parentWidget)
{
    auto *treeWidget = new QTreeWidget(parentWidget);
    treeWidget->setColumnCount(2);
    treeWidget->setHeaderLabels({QStringLiteral("Name"), QStringLiteral("Value")});

    auto *parentItem = new QTreeWidgetItem(treeWidget, {QStringLiteral("Parent"), QStringLiteral("A")});
    new QTreeWidgetItem(parentItem, {QStringLiteral("Child 1"), QStringLiteral("10")});
    new QTreeWidgetItem(parentItem, {QStringLiteral("Child 2"), QStringLiteral("20")});
    treeWidget->expandAll();
    treeWidget->setMinimumSize(320, 200);
    return treeWidget;
}

inline QWidget *createGroupBoxWidget(QWidget *parentWidget)
{
    auto *groupBox = new QGroupBox(QStringLiteral("Group Box"), parentWidget);
    groupBox->setCheckable(true);
    groupBox->setChecked(true);

    auto *layout = new QVBoxLayout(groupBox);
    layout->addWidget(new QCheckBox(QStringLiteral("Option 1"), groupBox));
    layout->addWidget(new QCheckBox(QStringLiteral("Option 2"), groupBox));
    groupBox->setMinimumSize(220, 120);
    return groupBox;
}

inline QWidget *createScrollAreaWidget(QWidget *parentWidget)
{
    auto *scrollArea = new QScrollArea(parentWidget);
    scrollArea->setWidgetResizable(true);

    auto *contentWidget = new QWidget(scrollArea);
    auto *contentLayout = new QVBoxLayout(contentWidget);
    for (int index = 0; index < 8; ++index) {
        contentLayout->addWidget(
            new QLabel(QStringLiteral("Scrollable content line %1").arg(index + 1), contentWidget));
    }
    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    scrollArea->setMinimumSize(280, 180);
    return scrollArea;
}

inline QWidget *createTabBarWidget(QWidget *parentWidget)
{
    auto *tabBar = new QTabBar(parentWidget);
    tabBar->addTab(QStringLiteral("General"));
    tabBar->addTab(QStringLiteral("Advanced"));
    tabBar->addTab(QStringLiteral("Preview"));
    tabBar->setCurrentIndex(1);
    tabBar->setMinimumSize(260, 36);
    return tabBar;
}

inline QWidget *createTabWidgetWidget(QWidget *parentWidget)
{
    auto *tabWidget = new QTabWidget(parentWidget);
    tabWidget->addTab(new QLabel(QStringLiteral("First page"), tabWidget), QStringLiteral("Page 1"));
    tabWidget->addTab(new QLabel(QStringLiteral("Second page"), tabWidget), QStringLiteral("Page 2"));
    tabWidget->setCurrentIndex(0);
    tabWidget->setMinimumSize(320, 200);
    return tabWidget;
}

inline QWidget *createStackedWidgetWidget(QWidget *parentWidget)
{
    auto *stackedWidget = new QStackedWidget(parentWidget);
    stackedWidget->addWidget(new QLabel(QStringLiteral("Stack page 1"), stackedWidget));
    stackedWidget->addWidget(new QLabel(QStringLiteral("Stack page 2"), stackedWidget));
    stackedWidget->setCurrentIndex(1);
    stackedWidget->setMinimumSize(260, 120);
    return stackedWidget;
}

inline QWidget *createFrameWidget(QWidget *parentWidget)
{
    auto *frame = new QFrame(parentWidget);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    auto *layout = new QVBoxLayout(frame);
    layout->addWidget(new QLabel(QStringLiteral("Frame content"), frame));
    frame->setMinimumSize(240, 120);
    return frame;
}

inline QWidget *createPlainWidget(QWidget *parentWidget)
{
    auto *widget = new QWidget(parentWidget);
    widget->setMinimumSize(240, 120);

    auto *layout = new QVBoxLayout(widget);
    layout->addWidget(new QLabel(QStringLiteral("QWidget demo"), widget));
    layout->addStretch();
    return widget;
}

inline QWidget *createCompositeWidget(QWidget *parentWidget)
{
    auto *compositeWidget = new QWidget(parentWidget);
    compositeWidget->setObjectName(QStringLiteral("compositeWidget"));
    compositeWidget->setMinimumSize(360, 240);

    auto *mainLayout = new QVBoxLayout(compositeWidget);

    auto *titleLabel = new QLabel(QStringLiteral("Composite Widget Demo"), compositeWidget);
    titleLabel->setObjectName(QStringLiteral("titleLabel"));
    titleLabel->setAlignment(Qt::AlignCenter);

    auto *nameLineEdit = new QLineEdit(QStringLiteral("Alice"), compositeWidget);
    nameLineEdit->setObjectName(QStringLiteral("nameLineEdit"));

    auto *enabledCheckBox = new QCheckBox(QStringLiteral("Enabled"), compositeWidget);
    enabledCheckBox->setObjectName(QStringLiteral("enabledCheckBox"));
    enabledCheckBox->setChecked(true);

    auto *valueSlider = new QSlider(Qt::Horizontal, compositeWidget);
    valueSlider->setObjectName(QStringLiteral("valueSlider"));
    valueSlider->setRange(0, 100);
    valueSlider->setValue(35);

    auto *buttonRow = new QWidget(compositeWidget);
    buttonRow->setObjectName(QStringLiteral("buttonRow"));
    auto *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    auto *applyButton = new QPushButton(QStringLiteral("Apply"), buttonRow);
    applyButton->setObjectName(QStringLiteral("applyButton"));
    auto *resetButton = new QPushButton(QStringLiteral("Reset"), buttonRow);
    resetButton->setObjectName(QStringLiteral("resetButton"));
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(resetButton);

    auto *detailsGroupBox = new QGroupBox(QStringLiteral("Details"), compositeWidget);
    detailsGroupBox->setObjectName(QStringLiteral("detailsGroupBox"));
    auto *detailsLayout = new QFormLayout(detailsGroupBox);

    auto *modeComboBox = new QComboBox(detailsGroupBox);
    modeComboBox->setObjectName(QStringLiteral("modeComboBox"));
    modeComboBox->addItems(
        {QStringLiteral("Standard"), QStringLiteral("Advanced"), QStringLiteral("Preview")});

    auto *countSpinBox = new QSpinBox(detailsGroupBox);
    countSpinBox->setObjectName(QStringLiteral("countSpinBox"));
    countSpinBox->setRange(0, 999);
    countSpinBox->setValue(12);

    detailsLayout->addRow(QStringLiteral("Mode"), modeComboBox);
    detailsLayout->addRow(QStringLiteral("Count"), countSpinBox);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(nameLineEdit);
    mainLayout->addWidget(enabledCheckBox);
    mainLayout->addWidget(valueSlider);
    mainLayout->addWidget(buttonRow);
    mainLayout->addWidget(detailsGroupBox);
    mainLayout->addStretch();
    return compositeWidget;
}

inline QWidget *createMdiAreaWidget(QWidget *parentWidget)
{
    auto *mdiArea = new QMdiArea(parentWidget);
    auto *documentWidget = new QTextEdit(QStringLiteral("MDI document"), mdiArea);
    auto *subWindow = mdiArea->addSubWindow(documentWidget);
    subWindow->resize(180, 100);
    documentWidget->show();
    mdiArea->setMinimumSize(320, 220);
    return mdiArea;
}

inline QWidget *createDockWidgetWidget(QWidget *parentWidget)
{
    auto *dockWidget = new QDockWidget(QStringLiteral("Dock Widget"), parentWidget);
    dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
	dockWidget->setFeatures(QDockWidget::DockWidgetClosable
							| QDockWidget::DockWidgetMovable
							| QDockWidget::DockWidgetFloatable);
    dockWidget->setWidget(new QTextEdit(QStringLiteral("Dock content"), dockWidget));
    dockWidget->setMinimumSize(320, 180);
    return dockWidget;
}

inline QWidget *createComboBoxWidget(QWidget *parentWidget)
{
    auto *comboBox = new QComboBox(parentWidget);
    comboBox->addItems({QStringLiteral("Alpha"), QStringLiteral("Beta"), QStringLiteral("Gamma")});
    comboBox->setCurrentIndex(1);
    comboBox->setMinimumWidth(220);
    return comboBox;
}

inline QWidget *createFontComboBoxWidget(QWidget *parentWidget)
{
    auto *fontComboBox = new QFontComboBox(parentWidget);
    fontComboBox->setMinimumWidth(260);
    return fontComboBox;
}

inline QWidget *createLineEditWidget(QWidget *parentWidget)
{
    auto *lineEdit = new QLineEdit(QStringLiteral("Line edit text"), parentWidget);
    lineEdit->setMinimumWidth(260);
    return lineEdit;
}

inline QWidget *createTextEditWidget(QWidget *parentWidget)
{
    auto *textEdit = new QTextEdit(parentWidget);
    textEdit->setPlainText(QStringLiteral("Multi-line\ntext edit\ndemo"));
    textEdit->setMinimumSize(320, 180);
    return textEdit;
}

inline QWidget *createPlainTextEditWidget(QWidget *parentWidget)
{
    auto *plainTextEdit = new QPlainTextEdit(parentWidget);
    plainTextEdit->setPlainText(QStringLiteral("Plain text edit\ndemo"));
    plainTextEdit->setMinimumSize(320, 180);
    return plainTextEdit;
}

inline QWidget *createSpinBoxWidget(QWidget *parentWidget)
{
    auto *spinBox = new QSpinBox(parentWidget);
    spinBox->setRange(-100, 1000);
    spinBox->setValue(42);
    spinBox->setMinimumWidth(160);
    return spinBox;
}

inline QWidget *createDoubleSpinBoxWidget(QWidget *parentWidget)
{
    auto *doubleSpinBox = new QDoubleSpinBox(parentWidget);
    doubleSpinBox->setRange(-1000.0, 1000.0);
    doubleSpinBox->setDecimals(3);
    doubleSpinBox->setValue(3.141);
    doubleSpinBox->setMinimumWidth(180);
    return doubleSpinBox;
}

inline QWidget *createTimeEditWidget(QWidget *parentWidget)
{
    auto *timeEdit = new QTimeEdit(QTime(12, 34, 56), parentWidget);
    timeEdit->setMinimumWidth(180);
    return timeEdit;
}

inline QWidget *createDateEditWidget(QWidget *parentWidget)
{
    auto *dateEdit = new QDateEdit(QDate(2026, 4, 7), parentWidget);
    dateEdit->setCalendarPopup(true);
    dateEdit->setMinimumWidth(180);
    return dateEdit;
}

inline QWidget *createDateTimeEditWidget(QWidget *parentWidget)
{
    auto *dateTimeEdit = new QDateTimeEdit(QDateTime(QDate(2026, 4, 7), QTime(9, 30, 0)),
                                           parentWidget);
    dateTimeEdit->setCalendarPopup(true);
    dateTimeEdit->setMinimumWidth(220);
    return dateTimeEdit;
}

inline QWidget *createDialWidget(QWidget *parentWidget)
{
    auto *dial = new QDial(parentWidget);
    dial->setRange(0, 100);
    dial->setValue(35);
    dial->setMinimumSize(140, 140);
    return dial;
}

inline QWidget *createScrollBarWidget(QWidget *parentWidget)
{
    auto *scrollBar = new QScrollBar(Qt::Horizontal, parentWidget);
    scrollBar->setRange(0, 100);
    scrollBar->setValue(40);
    scrollBar->setMinimumWidth(260);
    return scrollBar;
}

inline QWidget *createSliderWidget(QWidget *parentWidget)
{
    auto *slider = new QSlider(Qt::Horizontal, parentWidget);
    slider->setRange(0, 500);
    slider->setValue(120);
    slider->setMinimumWidth(260);
    return slider;
}

inline QWidget *createKeySequenceEditWidget(QWidget *parentWidget)
{
    auto *keySequenceEdit = new QKeySequenceEdit(QKeySequence(QStringLiteral("Ctrl+Shift+P")),
                                                 parentWidget);
    keySequenceEdit->setMinimumWidth(240);
    return keySequenceEdit;
}

inline QWidget *createLabelWidget(QWidget *parentWidget)
{
    auto *label = new QLabel(QStringLiteral("Label demo text"), parentWidget);
    label->setFrameShape(QFrame::Panel);
    label->setFrameShadow(QFrame::Sunken);
    label->setAlignment(Qt::AlignCenter);
    label->setMinimumSize(220, 80);
    return label;
}

inline QWidget *createTextBrowserWidget(QWidget *parentWidget)
{
    auto *textBrowser = new QTextBrowser(parentWidget);
    textBrowser->setHtml(QStringLiteral("<h3>Text Browser</h3><p><b>Rich text</b> preview.</p>"));
    textBrowser->setMinimumSize(320, 180);
    return textBrowser;
}

inline QWidget *createGraphicsViewWidget(QWidget *parentWidget)
{
    auto *graphicsView = new QGraphicsView(parentWidget);
    auto *scene = new QGraphicsScene(graphicsView);
    scene->addRect(0, 0, 120, 80);
    scene->addText(QStringLiteral("Graphics View"));
    graphicsView->setScene(scene);
    graphicsView->setMinimumSize(320, 220);
    return graphicsView;
}

inline QWidget *createCalendarWidgetWidget(QWidget *parentWidget)
{
    auto *calendarWidget = new QCalendarWidget(parentWidget);
    calendarWidget->setSelectedDate(QDate(2026, 4, 7));
    calendarWidget->setMinimumSize(320, 220);
    return calendarWidget;
}

inline QWidget *createLcdNumberWidget(QWidget *parentWidget)
{
    auto *lcdNumber = new QLCDNumber(parentWidget);
    lcdNumber->setDigitCount(6);
    lcdNumber->display(123456);
    lcdNumber->setMinimumSize(220, 80);
    return lcdNumber;
}

inline QWidget *createProgressBarWidget(QWidget *parentWidget)
{
    auto *progressBar = new QProgressBar(parentWidget);
    progressBar->setRange(0, 100);
    progressBar->setValue(65);
    progressBar->setMinimumWidth(260);
    return progressBar;
}

inline QWidget *createOpenGlWidget(QWidget *parentWidget)
{
#if DEMO_HAS_OPENGL_WIDGET
    auto *openGlWidget = new QOpenGLWidget(parentWidget);
    openGlWidget->setMinimumSize(320, 220);
    return openGlWidget;
#else
    return createUnavailableModuleWidget(QStringLiteral("QOpenGLWidget"), parentWidget);
#endif
}

inline QWidget *createQuickWidgetWidget(QWidget *parentWidget)
{
#if DEMO_HAS_QUICK_WIDGET
    auto *quickWidget = new QQuickWidget(parentWidget);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setMinimumSize(320, 220);
    return quickWidget;
#else
    return createUnavailableModuleWidget(QStringLiteral("QQuickWidget"), parentWidget);
#endif
}

inline QWidget *createWebEngineViewWidget(QWidget *parentWidget)
{
#if DEMO_HAS_WEBENGINE_VIEW
    auto *webEngineView = new QWebEngineView(parentWidget);
    webEngineView->setHtml(QStringLiteral("<html><body><h3>QWebEngineView</h3><p>Demo page.</p></body></html>"));
    webEngineView->setMinimumSize(360, 240);
    return webEngineView;
#else
    return createUnavailableModuleWidget(QStringLiteral("QWebEngineView"), parentWidget);
#endif
}

inline QVector<DemoWidgetSpecification> createDemoWidgetSpecifications()
{
    QVector<DemoWidgetSpecification> widgetSpecifications = {
        {QStringLiteral("Composite Widget"), createCompositeWidget},
        {QStringLiteral("QCalendarWidget"), createCalendarWidgetWidget},
        {QStringLiteral("QCheckBox"), createCheckBoxWidget},
        {QStringLiteral("QComboBox"), createComboBoxWidget},
        {QStringLiteral("QCommandLinkButton"), createCommandLinkButtonWidget},
        {QStringLiteral("QDateEdit"), createDateEditWidget},
        {QStringLiteral("QDateTimeEdit"), createDateTimeEditWidget},
        {QStringLiteral("QDial"), createDialWidget},
        {QStringLiteral("QDialogButtonBox"), createDialogButtonBoxWidget},
        {QStringLiteral("QDockWidget"), createDockWidgetWidget},
        {QStringLiteral("QDoubleSpinBox"), createDoubleSpinBoxWidget},
        {QStringLiteral("QFontComboBox"), createFontComboBoxWidget},
        {QStringLiteral("QFrame"), createFrameWidget},
        {QStringLiteral("QGraphicsView"), createGraphicsViewWidget},
        {QStringLiteral("QGroupBox"), createGroupBoxWidget},
        {QStringLiteral("QKeySequenceEdit"), createKeySequenceEditWidget},
        {QStringLiteral("QLCDNumber"), createLcdNumberWidget},
        {QStringLiteral("QLabel"), createLabelWidget},
        {QStringLiteral("QLineEdit"), createLineEditWidget},
        {QStringLiteral("QListView"), createListViewWidget},
        {QStringLiteral("QListWidget"), createListWidgetWidget},
        {QStringLiteral("QMdiArea"), createMdiAreaWidget},
        {QStringLiteral("QOpenGLWidget"), createOpenGlWidget},
        {QStringLiteral("QPlainTextEdit"), createPlainTextEditWidget},
        {QStringLiteral("QProgressBar"), createProgressBarWidget},
        {QStringLiteral("QPushButton"), createPushButtonWidget},
        {QStringLiteral("QQuickWidget"), createQuickWidgetWidget},
        {QStringLiteral("QRadioButton"), createRadioButtonWidget},
        {QStringLiteral("QScrollArea"), createScrollAreaWidget},
        {QStringLiteral("QScrollBar"), createScrollBarWidget},
        {QStringLiteral("QSlider"), createSliderWidget},
        {QStringLiteral("QSpinBox"), createSpinBoxWidget},
        {QStringLiteral("QStackedWidget"), createStackedWidgetWidget},
        {QStringLiteral("QTabBar"), createTabBarWidget},
        {QStringLiteral("QTabWidget"), createTabWidgetWidget},
        {QStringLiteral("QTableView"), createTableViewWidget},
        {QStringLiteral("QTableWidget"), createTableWidgetWidget},
        {QStringLiteral("QTextBrowser"), createTextBrowserWidget},
        {QStringLiteral("QTextEdit"), createTextEditWidget},
        {QStringLiteral("QTimeEdit"), createTimeEditWidget},
        {QStringLiteral("QToolButton"), createToolButtonWidget},
        {QStringLiteral("QTreeView"), createTreeViewWidget},
        {QStringLiteral("QTreeWidget"), createTreeWidgetWidget},
        {QStringLiteral("QWebEngineView"), createWebEngineViewWidget},
        {QStringLiteral("QWidget"), createPlainWidget}
    };

    std::sort(widgetSpecifications.begin(), widgetSpecifications.end(),
              [](const DemoWidgetSpecification &leftHandSide,
                 const DemoWidgetSpecification &rightHandSide) {
                  return QString::localeAwareCompare(leftHandSide.className,
                                                     rightHandSide.className)
                      < 0;
              });

    return widgetSpecifications;
}

inline void setupDemoWindow(QWidget *windowWidget)
{
    auto *horizontalLayout = new QHBoxLayout(windowWidget);

    auto *previewPanel = new QWidget(windowWidget);
    auto *previewLayout = new QVBoxLayout(previewPanel);

    auto *widgetSelector = new QComboBox(previewPanel);
    auto *showObjectTreeButton = new QPushButton(QStringLiteral("Show Object Tree"), previewPanel);
    auto *pickObjectButton = new QPushButton(QStringLiteral("Pick Object"), previewPanel);
    widgetSelector->setFocusPolicy(Qt::StrongFocus);
    widgetSelector->setFocus();
    auto *previewAreaWidget = new QWidget(previewPanel);
    auto *previewAreaLayout = new QVBoxLayout(previewAreaWidget);
    auto *propertyEditorHost = new PropertyEditor(windowWidget);
    auto *objectTreeDialog = new ObjectTreeDialog(windowWidget);
    auto *objectPicker = new ObjectPicker(windowWidget);

    previewAreaLayout->setContentsMargins(0, 0, 0, 0);
    previewAreaWidget->setMinimumSize(420, 320);

    previewLayout->addWidget(widgetSelector, 0, Qt::AlignHCenter);
    previewLayout->addWidget(showObjectTreeButton, 0, Qt::AlignHCenter);
    previewLayout->addWidget(pickObjectButton, 0, Qt::AlignHCenter);
    previewLayout->addWidget(previewAreaWidget, 1);

    horizontalLayout->addWidget(previewPanel, 1);
    horizontalLayout->addWidget(propertyEditorHost, 1);

    const QVector<DemoWidgetSpecification> widgetSpecifications = createDemoWidgetSpecifications();
    for (const DemoWidgetSpecification &widgetSpecification : widgetSpecifications) {
        widgetSelector->addItem(widgetSpecification.className);
    }

    auto currentDisplayedWidget = std::make_shared<QPointer<QWidget>>();
    const auto updateDisplayedWidget =
        [widgetSpecifications, previewAreaLayout, propertyEditorHost, objectTreeDialog, objectPicker,
         currentDisplayedWidget](int currentIndex) {
            if (currentIndex < 0 || currentIndex >= widgetSpecifications.size()) {
                propertyEditorHost->setInspectedObject(nullptr);
                objectTreeDialog->setCurrentObject(nullptr);
                objectPicker->setSelectedObject(nullptr);
                return;
            }

            if (*currentDisplayedWidget != nullptr) {
                previewAreaLayout->removeWidget(*currentDisplayedWidget);
                (*currentDisplayedWidget)->deleteLater();
                *currentDisplayedWidget = nullptr;
            }

            QWidget *displayedWidget = widgetSpecifications.at(currentIndex).createWidget(
                previewAreaLayout->parentWidget());
            previewAreaLayout->addWidget(displayedWidget, 0, Qt::AlignCenter);
            *currentDisplayedWidget = displayedWidget;
            propertyEditorHost->setInspectedObject(displayedWidget);
            objectTreeDialog->setCurrentObject(displayedWidget);
            objectPicker->setSelectedObject(displayedWidget);
        };

    QObject::connect(widgetSelector, qOverload<int>(&QComboBox::currentIndexChanged), windowWidget,
                     updateDisplayedWidget);
    QObject::connect(objectTreeDialog->objectTreeWidget(), &ObjectTreeWidget::objectActivated,
                     windowWidget, [propertyEditorHost, objectPicker](QObject *targetObject) {
                         propertyEditorHost->setInspectedObject(targetObject);
                         objectPicker->setSelectedObject(targetObject);
                     });
    QObject::connect(showObjectTreeButton, &QPushButton::clicked, windowWidget,
                     [propertyEditorHost, objectPicker, objectTreeDialog, currentDisplayedWidget]() {
                         objectPicker->stop();
                         objectTreeDialog->setRootObject(currentDisplayedWidget->data());
                         objectTreeDialog->setCurrentObject(propertyEditorHost->inspectedObject());
                         objectTreeDialog->show();
                         objectTreeDialog->raise();
                         objectTreeDialog->activateWindow();
                     });
    QObject::connect(pickObjectButton, &QPushButton::clicked, windowWidget,
                     [objectPicker, objectTreeDialog, currentDisplayedWidget]() {
                         QWidget *rootWidget = currentDisplayedWidget->data();
                         if (rootWidget == nullptr || !rootWidget->isVisible()) {
                             return;
                         }

                         if (objectTreeDialog->isVisible()) {
                             objectTreeDialog->hide();
                         }

                         objectPicker->start(rootWidget);
                     });
    QObject::connect(objectPicker, &ObjectPicker::objectPicked, windowWidget,
                     [propertyEditorHost](QObject *targetObject) {
                         propertyEditorHost->setInspectedObject(targetObject);
                     });

    if (!widgetSpecifications.isEmpty()) {
        updateDisplayedWidget(0);
    }

    windowWidget->resize(1200, 760);
}

} // namespace DemoWidgetSelector
