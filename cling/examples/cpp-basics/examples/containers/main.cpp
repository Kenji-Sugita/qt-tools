#include <QCoreApplication>
#include <QMap>
#include <QStringList>
#include <QDebug>
#include <QPoint>

bool operator<(const QPoint& point1, const QPoint& point2)
{
    return (point1.x() < point2.x() || (point1.x() == point2.x() && point1.y() < point2.y()));
}

uint qHash(const QPoint& point)
{
    return qHash(point.x()) ^ qHash(point.y());
}

int main(int argc, char** argv)
{
    QCoreApplication(argc, argv);

    QMap<QPoint, int> map { { QPoint(1, 2), 42 } };
    Q_ASSERT(map.value(QPoint(1, 2)) == 42);

    QHash<QPoint, int > hash { { QPoint(1, 1), 42 } };
    Q_ASSERT(hash.value(QPoint(1, 1)) == 42);

    QSet<QPoint> set { { QPoint(1, 1) } };
    Q_ASSERT(set.contains(QPoint(1, 1)));

    QStringList strings { "abc", "def", "hij" };

    QMutableListIterator<QString> it(strings);
    it.toBack();
    qDebug() << "Just iterated over: " << it.previous();
    it.insert("Foo");
    qDebug() << "Next item after the insert is: " << it.previous();

    it = QMutableListIterator<QString>(strings);
    qDebug() << "Just iterated over: " << it.next();
    it.insert("Bar");
    qDebug() << "Next item after the insert is: " << it.next();

    it = QMutableListIterator<QString>(strings);
    while (it.hasNext()) {
        qDebug() << it.next();
    }
}
