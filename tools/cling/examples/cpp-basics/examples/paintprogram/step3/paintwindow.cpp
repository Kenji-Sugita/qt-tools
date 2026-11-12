#include "paintwindow.h"
#include "scribblearea.h"

#include <QMetaType>
Q_DECLARE_METATYPE(Qt::PenStyle)

#include <QFileDialog>
#include <QScrollArea>
#include <QMessageBox>
#include <QActionGroup>
#include <QToolBar>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>

namespace {
    const char* const fileFilter = "Images(*.png *.jpg *.xpm)";
}

PaintWindow::PaintWindow(QWidget* parent)
    : QMainWindow(parent)
{
    const auto scroll = new QScrollArea(this);
    setCentralWidget(scroll);
    scribbleArea = new ScribbleArea(scroll);
    scribbleArea->resize(2000, 2000);
    scroll->setWidget(scribbleArea);

    setupFileMenu();
    setupColorMenu();
    setupPenWidthMenu();
    setupPenStyleMenu();
    setupHelpMenu();

    statusBar()->showMessage("Ready!", 3000);
}

void PaintWindow::setupFileMenu()
{
    const auto loadAction = new QAction(QPixmap(":/icons/load.png"), "&Load", this);
    loadAction->setShortcut(Qt::CTRL | Qt::Key_L);
    connect(loadAction, &QAction::triggered, this, &PaintWindow::slotLoad);

    const auto saveAction = new QAction( QPixmap(":/icons/save.png"), "&Save", this);
    saveAction->setShortcut(Qt::CTRL | Qt::Key_S);
    connect(saveAction, &QAction::triggered, this, &PaintWindow::slotSave);

    const auto printPreviewAction = new QAction( QPixmap(), "Print Pre&view", this);
    connect(printPreviewAction, &QAction::triggered, scribbleArea, &ScribbleArea::slotPrintPreview);

    const auto printAction = new QAction( QPixmap(":/icons/print.png"), "&Print", this);
    printAction->setShortcut(Qt::CTRL | Qt::Key_P);
    connect(printAction, &QAction::triggered, scribbleArea, &ScribbleArea::slotPrint);

    const auto quitAction = new QAction(QPixmap(":/icons/quit.png"), "&Quit", this);
    quitAction->setShortcut(Qt::CTRL | Qt::Key_Q);
    quitAction->setStatusTip("This will quit the application unconditionally");
    quitAction->setWhatsThis("This is a button, stupid!");
    connect(quitAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);

    const auto fileMenu = new QMenu("&File", menuBar());
    menuBar()->addMenu(fileMenu);
    fileMenu->addAction(loadAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(printPreviewAction);
    fileMenu->addAction(printAction);
    fileMenu->addAction(quitAction);

    const auto mainToolBar = new QToolBar("Main Toolbar", this);
    addToolBar(mainToolBar);
    mainToolBar->addAction(loadAction);
    mainToolBar->addAction(saveAction);
    mainToolBar->addAction(printAction);
    mainToolBar->addAction(quitAction);
}

void PaintWindow::setupColorMenu()
{
    const auto actions = new QActionGroup(this);
    actions->setExclusive(true);
    connect(actions, &QActionGroup::triggered, this, &PaintWindow::slotChangeColor);
    connect(this, &PaintWindow::colorChanged, scribbleArea, &ScribbleArea::slotChangeColor);

    const auto black = new QAction(QPixmap(":/icons/black.png"), "black", actions);
    black->setCheckable(true);
    black->setChecked(true);
    black->setData(QColor(Qt::black));

    const auto red = new QAction(QPixmap(":/icons/red.png"), "red", actions);
    red->setCheckable(true);
    red->setData(QColor(Qt::red));

    const auto blue = new QAction(QPixmap(":/icons/blue.png"), "blue", actions);
    blue->setCheckable(true);
    blue->setData(QColor(Qt::blue));

    const auto green = new QAction(QPixmap(":/icons/green.png"), "green", actions);
    green->setCheckable(true);
    green->setData(QColor(Qt::green));

    const auto yellow = new QAction(QPixmap(":/icons/yellow.png"), "yellow", actions);
    yellow->setCheckable(true);
    yellow->setData(QColor(Qt::yellow));

    const auto colorMenu = new QMenu("&Colors", menuBar());
    menuBar()->addMenu(colorMenu);
    colorMenu->addActions(actions->actions());

    const auto colorToolBar = new QToolBar("Colors", this);
    addToolBar(colorToolBar);
    colorToolBar->addActions(actions->actions());
}

void PaintWindow::setupPenWidthMenu()
{
    const auto actions = new QActionGroup(this);
    actions->setExclusive(true);
    connect(actions, &QActionGroup::triggered, this, &PaintWindow::slotChangePenWidth);

    const auto one = new QAction(QPixmap(":/icons/1.png"), "1px",  actions);
    one->setCheckable(true);
    one->setData(1);

    const auto two = new QAction(QPixmap(":/icons/2.png"), "2px",  actions);
    two->setCheckable(true);
    two->setData(2);

    const auto three = new QAction(QPixmap(":/icons/3.png"), "3px",  actions);
    three->setCheckable(true);
    three->setData(3);
    three->setChecked(true);

    const auto penWidthMenu = new QMenu("&Width", menuBar());
    menuBar()->addMenu(penWidthMenu);
    penWidthMenu->addActions(actions->actions());

    const auto penWidthBar = new QToolBar("Pen Widths", this);
    addToolBar(penWidthBar);
    penWidthBar->addActions(actions->actions());
}

void PaintWindow::setupPenStyleMenu()
{
    const auto actions = new QActionGroup(this);
    actions->setExclusive(true);
    connect(actions, &QActionGroup::triggered, this, &PaintWindow::slotChangePenStyle);

    const auto solid  = new QAction(QPixmap(":/icons/solid.xpm"), "solid", actions);
    solid->setCheckable(true);
    solid->setChecked(true);
    solid->setData(QVariant::fromValue(Qt::SolidLine));

    const auto dash = new QAction(QPixmap(":/icons/dash.xpm"), "dash", actions);
    dash->setCheckable(true);
    dash->setData(QVariant::fromValue(Qt::DashLine));

    const auto dot = new QAction(QPixmap(":/icons/dot.xpm"), "dot", actions);
    dot->setCheckable(true);
    dot->setData(QVariant::fromValue(Qt::DotLine));

    const auto dashdot = new QAction(QPixmap(":/icons/dashdot.xpm"), "dashdot", actions);
    dashdot->setCheckable(true);
    dashdot->setData(QVariant::fromValue(Qt::DashDotLine));

    const auto dashdotdot = new QAction(QPixmap(":/icons/dashdotdot.xpm"), "dashdotdot", actions);
    dashdotdot->setCheckable(true);
    dashdotdot->setData(QVariant::fromValue(Qt::DashDotDotLine));

    const auto penStyleMenu = new QMenu("&Style", menuBar());
    menuBar()->addMenu(penStyleMenu);
    penStyleMenu->addActions(actions->actions());

    const auto penStyleToolBar = new QToolBar("Pen Styles", this);
    addToolBar(penStyleToolBar);
    penStyleToolBar->addActions(actions->actions());
}

void PaintWindow::setupHelpMenu()
{
    const auto about = new QAction("&About This Program", this);
    connect(about, &QAction::triggered, this, &PaintWindow::slotAbout);

    const auto aboutQt = new QAction("About &Qt", this);
    connect(aboutQt, &QAction::triggered, this, &PaintWindow::slotAboutQt);

    const auto helpMenu = new QMenu("&Help", menuBar());
    menuBar()->addSeparator();
    menuBar()->addMenu(helpMenu);
    helpMenu->addAction(about);
    helpMenu->addAction(aboutQt);
}

void PaintWindow::slotChangeColor(QAction* action)
{
    emit colorChanged(action->data().value<QColor>());
}

void PaintWindow::slotChangePenWidth(QAction* action)
{
    scribbleArea->setPenWidth(action->data().toInt());
}

void PaintWindow::slotChangePenStyle(QAction* action)
{
    scribbleArea->setPenStyle(action->data().value<Qt::PenStyle>());
}

void PaintWindow::slotAbout()
{
    QMessageBox::information(this, "About The PaintWindow Program",
                             "This is the paint program\n"
                             "(C) 2000-2006 by Matthias Kalle Dalheimer\n"
                             "and Jesper K. Pedersen");
}

void PaintWindow::slotAboutQt()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void PaintWindow::slotLoad()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Image", ".", fileFilter);
    if (!filename.isEmpty()) {
        scribbleArea->slotLoad(filename);
    }
}

void PaintWindow::slotSave()
{
    QString filename = QFileDialog::getSaveFileName(this, QString(), QString(), fileFilter);
    if (!filename.isEmpty()) {
        scribbleArea->slotSave(filename);
    }
}
