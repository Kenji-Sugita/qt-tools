#include "scribblearea.h"
#include "paintwindow.h"

#include <QMessageBox>
#include <QActionGroup>
#include <QToolBar>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>

PaintWindow::PaintWindow(QWidget* parent)
    : QMainWindow(parent)
{
    scribbleArea = new ScribbleArea(this);
    setCentralWidget(scribbleArea);

    setupFileMenu();
    setupColorMenu();
    setupHelpMenu();

    statusBar()->showMessage("Ready!", 3000);
}

void PaintWindow::setupFileMenu()
{
    const auto quitAction = new QAction(QPixmap("icons/quit.png"), "&Quit", this);
    quitAction->setShortcut(Qt::CTRL | Qt::Key_Q);
    quitAction->setStatusTip("This will quit the application unconditionally");
    quitAction->setWhatsThis("This is a button, stupid!");
    connect(quitAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);

    const auto fileMenu = new QMenu("&File", menuBar());
    menuBar()->addMenu(fileMenu);
    fileMenu->addAction(quitAction);

    const auto mainToolBar = new QToolBar("Main Toolbar", this);
    addToolBar(mainToolBar);
    mainToolBar->addAction(quitAction);
}


void PaintWindow::setupColorMenu()
{
    const auto actions = new QActionGroup(this);
    connect(actions, &QActionGroup::triggered, this, &PaintWindow::slotChangeColor);
    actions->setExclusive(true);

    const auto black = new QAction(QPixmap("icons/black.png"), "black", actions);
    black->setCheckable(true);
    black->setChecked(true);
    black->setData(QColor(Qt::black));

    const auto red = new QAction(QPixmap("icons/red.png"), "red", actions);
    red->setCheckable(true);
    red->setData(QColor(Qt::red));

    const auto blue = new QAction(QPixmap("icons/blue.png"), "blue", actions);
    blue->setCheckable(true);
    blue->setData(QColor(Qt::blue));

    const auto green = new QAction(QPixmap("icons/green.png"), "green", actions);
    green->setCheckable(true);
    green->setData(QColor(Qt::green));

    const auto yellow = new QAction(QPixmap("icons/yellow.png"), "yellow", actions);
    yellow->setCheckable(true);
    yellow->setData(QColor(Qt::yellow));

    const auto colorMenu = new QMenu("&Colors", menuBar());
    menuBar()->addMenu(colorMenu);
    colorMenu->addActions(actions->actions());

    const auto colorToolBar = new QToolBar("Colors", this);
    addToolBar(colorToolBar);
    colorToolBar->addActions(actions->actions());
}

void PaintWindow::setupHelpMenu()
{
    const auto about = new QAction("&About This Program", this);
    connect(about, &QAction::triggered, this, &PaintWindow::slotAbout);

    const auto aboutQt = new QAction("About &Qt", this);
    connect(aboutQt, &QAction::triggered, this, &PaintWindow::slotAboutQt);

    const auto helpMenu = new QMenu("&Help", menuBar());
    menuBar()->addMenu(helpMenu);
    helpMenu->addAction(about);
    helpMenu->addAction(aboutQt);
}

void PaintWindow::slotChangeColor(QAction* action)
{
    scribbleArea->slotChangeColor(action->data().value<QColor>());
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
