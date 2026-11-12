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

protected:
    void setupFileMenu();
    void setupColorMenu();
    void setupPenWidthMenu();
    void setupPenStyleMenu();
    void setupHelpMenu();

protected slots:
    void slotChangeColor(QAction* action);
    void slotChangePenWidth(QAction* action);
    void slotChangePenStyle(QAction* action);
    void slotAbout();
    void slotAboutQt();
    void slotLoad();
    void slotSave();

signals:
    void colorChanged(const QColor& color);

private:
    ScribbleArea* scribbleArea;
};
#endif
