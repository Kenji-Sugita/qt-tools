#ifndef PAINTWINDOW_H
#define PAINTWINDOW_H

#include <QMainWindow>

class QAction;
class ScribbleArea;

class PaintWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PaintWindow(QWidget* parent = nullptr);

private:
    void setupFileMenu();
    void setupColorMenu();
    void setupHelpMenu();

private slots:
    void slotChangeColor(QAction* action);
    void slotAbout();
    void slotAboutQt();

private:
    ScribbleArea* scribbleArea;
};
#endif
