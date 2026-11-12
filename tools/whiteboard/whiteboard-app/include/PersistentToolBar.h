#pragma once

#include <QHash>
#include <QPointer>
#include <QSize>
#include <QVector>
#include <QWidget>

class QAction;
class QFrame;
class QHBoxLayout;
class QHideEvent;
class QLayout;
class QResizeEvent;
class QToolButton;
class QVBoxLayout;

class PersistentToolBar : public QWidget {
    Q_OBJECT

public:
    explicit PersistentToolBar(QWidget *overlayParent, QWidget *parent = nullptr);

    void addAction(QAction *action);
    void addSeparator();
    void addWidget(QWidget *widget);
    QWidget *widgetForAction(QAction *action) const;
    void setIconSize(const QSize &iconSize);
    void setOverflowStyleSheet(const QString &styleSheet);

    bool hasOverflow() const;
    bool overflowExpanded() const;
    void setOverflowExpanded(bool expanded);
    void refreshOverflow();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct Item {
        QWidget *widget = nullptr;
        bool separator = false;
    };

    void clearLayout(QLayout *layout);
    void configureItemWidget(QWidget *widget);
    int itemWidth(const Item &item) const;
    int overflowPanelLeftInset() const;
    void populateOverflowPanel(int firstOverflowIndex);
    void positionOverflowPanel();
    void updateExtensionButton();

    QPointer<QWidget> m_overlayParent;
    QHBoxLayout *m_primaryLayout = nullptr;
    QFrame *m_overflowPanel = nullptr;
    QVBoxLayout *m_overflowLayout = nullptr;
    QToolButton *m_extensionButton = nullptr;
    QVector<Item> m_items;
    QHash<QAction *, QWidget *> m_actionWidgets;
    QSize m_iconSize = QSize(24, 24);
    int m_firstOverflowIndex = -1;
    bool m_refreshing = false;
};
