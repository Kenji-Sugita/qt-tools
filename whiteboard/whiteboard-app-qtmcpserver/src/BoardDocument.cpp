#include "BoardDocument.h"

#include <QJsonArray>

QVector<Page> BoardDocument::pages() const
{
    return m_pages;
}

void BoardDocument::setPages(const QVector<Page> &pages)
{
    m_pages = pages.isEmpty() ? QVector<Page>{Page()} : pages.mid(0, MaxPages);
    m_currentPageIndex = qBound(0, m_currentPageIndex, m_pages.size() - 1);
}

int BoardDocument::currentPageIndex() const
{
    return m_currentPageIndex;
}

void BoardDocument::setCurrentPageIndex(int currentPageIndex)
{
    m_currentPageIndex = qBound(0, currentPageIndex, m_pages.size() - 1);
}

BoardSettings BoardDocument::settings() const
{
    return m_settings;
}

void BoardDocument::setSettings(const BoardSettings &settings)
{
    m_settings = settings;
}

Page &BoardDocument::currentPage()
{
    return m_pages[m_currentPageIndex];
}

const Page &BoardDocument::currentPage() const
{
    return m_pages[m_currentPageIndex];
}

QJsonObject BoardDocument::toJson() const
{
    QJsonArray pages;
    for (const Page &page : m_pages)
        pages.append(page.toJson());

    return {
        {QStringLiteral("pages"), pages},
        {QStringLiteral("currentPageIndex"), m_currentPageIndex},
        {QStringLiteral("settings"), m_settings.toJson()},
    };
}

BoardDocument BoardDocument::fromJson(const QJsonObject &object)
{
    BoardDocument document;
    const BoardSettings settings = BoardSettings::fromJson(object.value(QStringLiteral("settings")).toObject());
    QVector<Page> pages;
    const QJsonArray array = object.value(QStringLiteral("pages")).toArray();
    pages.reserve(array.size());
    for (const QJsonValue &value : array) {
        const QJsonObject pageObject = value.toObject();
        Page page = Page::fromJson(pageObject);
        if (!pageObject.contains(QStringLiteral("zoomPercent")))
            page.setZoomPercent(settings.zoomPercent());
        pages.append(page);
    }
    document.setPages(pages);
    document.setSettings(settings);
    document.setCurrentPageIndex(object.value(QStringLiteral("currentPageIndex")).toInt(0));
    return document;
}
