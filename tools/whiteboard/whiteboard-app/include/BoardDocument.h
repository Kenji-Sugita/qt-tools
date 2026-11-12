#pragma once

#include "BoardSettings.h"
#include "Page.h"

#include <QJsonObject>
#include <QtGlobal>
#include <QString>
#include <QVector>

class BoardDocument {
public:
    static constexpr int MaxPages = 20;

    QVector<Page> pages() const;
    void setPages(const QVector<Page> &pages);

    int currentPageIndex() const;
    void setCurrentPageIndex(int currentPageIndex);

    BoardSettings settings() const;
    void setSettings(const BoardSettings &settings);

    int nextElementId() const;
    void setNextElementId(int nextElementId);
    QString allocateElementId();
    void ensureElementIds();

    qint64 revision() const;
    void setRevision(qint64 revision);
    void advanceRevision();

    Page &currentPage();
    const Page &currentPage() const;

    QJsonObject toJson() const;
    static BoardDocument fromJson(const QJsonObject &object);

private:
    QVector<Page> m_pages = {Page()};
    int m_currentPageIndex = 0;
    BoardSettings m_settings;
    int m_nextElementId = 1;
    qint64 m_revision = 0;
};
