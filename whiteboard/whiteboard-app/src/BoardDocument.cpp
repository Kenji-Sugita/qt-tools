#include "BoardDocument.h"

#include <QHash>
#include <QJsonArray>
#include <QSet>
#include <QtGlobal>

#include <limits>

namespace {
int numericElementIdValue(const QString &id)
{
    static const QString prefix = QStringLiteral("element-");
    if (!id.startsWith(prefix))
        return 0;
    bool ok = false;
    const int value = id.mid(prefix.size()).toInt(&ok);
    return ok && value > 0 ? value : 0;
}
}

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

int BoardDocument::nextElementId() const
{
    return m_nextElementId;
}

void BoardDocument::setNextElementId(int nextElementId)
{
    m_nextElementId = qMax(1, nextElementId);
}

QString BoardDocument::allocateElementId()
{
    QSet<QString> usedIds;
    for (const Page &page : m_pages) {
        for (const DrawingElement &element : page.elements()) {
            if (!element.id().isEmpty())
                usedIds.insert(element.id());
        }
    }

    QString id;
    do {
        id = QStringLiteral("element-%1").arg(m_nextElementId++);
    } while (usedIds.contains(id));
    return id;
}

void BoardDocument::ensureElementIds()
{
    QHash<QString, int> idCounts;
    for (const Page &page : m_pages) {
        for (const DrawingElement &element : page.elements()) {
            const QString id = element.id().trimmed();
            if (!id.isEmpty())
                ++idCounts[id];
        }
    }

    QSet<QString> usedIds;
    int maxNumericId = 0;
    int nextCandidate = qMax(1, m_nextElementId);

    auto allocateUnused = [&idCounts, &usedIds, &nextCandidate, &maxNumericId]() {
        QString id;
        do {
            id = QStringLiteral("element-%1").arg(nextCandidate++);
        } while (usedIds.contains(id) || idCounts.contains(id));
        usedIds.insert(id);
        maxNumericId = qMax(maxNumericId, numericElementIdValue(id));
        return id;
    };

    for (Page &page : m_pages) {
        QVector<DrawingElement> elements = page.elements();
        bool changed = false;
        for (DrawingElement &element : elements) {
            const QString currentId = element.id().trimmed();
            if (!currentId.isEmpty() && idCounts.value(currentId) == 1 && !usedIds.contains(currentId)) {
                if (currentId != element.id()) {
                    element.setId(currentId);
                    changed = true;
                }
                usedIds.insert(currentId);
                maxNumericId = qMax(maxNumericId, numericElementIdValue(currentId));
                continue;
            }
            element.setId(allocateUnused());
            changed = true;
        }
        if (changed)
            page.setElements(elements);
    }

    m_nextElementId = qMax(nextCandidate, maxNumericId + 1);
}

qint64 BoardDocument::revision() const
{
    return m_revision;
}

void BoardDocument::setRevision(qint64 revision)
{
    m_revision = qMax<qint64>(0, revision);
}

void BoardDocument::advanceRevision()
{
    if (m_revision < std::numeric_limits<qint64>::max())
        ++m_revision;
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
        {QStringLiteral("nextElementId"), m_nextElementId},
        {QStringLiteral("revision"), m_revision},
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
    document.setNextElementId(object.value(QStringLiteral("nextElementId")).toInt(1));
    document.setRevision(object.value(QStringLiteral("revision")).toInteger(0));
    document.ensureElementIds();
    document.setSettings(settings);
    document.setCurrentPageIndex(object.value(QStringLiteral("currentPageIndex")).toInt(0));
    return document;
}
