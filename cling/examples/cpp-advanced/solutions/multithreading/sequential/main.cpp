#include <QGuiApplication>
#include <QtConcurrent>
#include <QRegion>
#include <QPolygonF>
#include <QPainter>
#include <QPainterPath>

static const int ITERATIONS = 1;        // Adjust to your computer's speed and RAM
static const bool SEQUENTIAL = false;   // Set to false to test your code
static const bool OPTIONALPART = true; // Set to true to enable the optional part

template<class T>
using Container = QList<T>;
//using Container = QVector<T>;

// Test data generators, defined towards the end (but not interesting):
static Container<double> makeDoubles();
static Container<QString> makeStrings(const Container<double>&);
static Container<QRect> makeRectangles(const Container<double>&);
static Container<QPolygonF> makePolygons(const Container<double>&);

//
// Task 1: Filtering/grepping strings
//

static bool isFirstLetterLowerCase(const QString& s)
{
    return !s.isEmpty() && s.at(0).isLower();
}

static Container<QString> grepFirstLetterLowerCase(const Container<QString>& c)
{
    if (SEQUENTIAL) {
        Container<QString> result;
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        for (const QString& s : std::as_const(c)) {
#else
        for (const QString& s : qAsConst(c)) {
#endif
            if (isFirstLetterLowerCase(s)) {
                result.push_back(s);
            }
        }
        return result;
    } else {
        // ### Implement here
        return QtConcurrent::blockingFiltered(c, isFirstLetterLowerCase);
    }
}

static void grepFirstLetterLowerCaseInline(Container<QString>& c)
{
    QtConcurrent::blockingFilter(c, isFirstLetterLowerCase);
}

//
// Task 2: Calculating some complex formula
//

static double formula(double x)
{
    return std::exp(-std::abs(x)) + std::sin(x) + std::cos(x);
}

static Container<double> calculateFormula(const Container<double>& c)
{
    if (SEQUENTIAL) {
        Container<double> result;
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        for (double v : std::as_const(c)) {
#else
        for (double v : qAsConst(c)) {
#endif
            result.push_back(formula(v));
        }
        return result;
    } else {
        // ### Implement here
        return QtConcurrent::blockingMapped(c, formula);
    }
}

static void formulaInline(double& x)
{
    x = formula(x);
}

static void calculateFormulaInline(Container<double>& c)
{
    // ### Optionally implement here
    QtConcurrent::blockingMap(c, formulaInline);
}

//
// Task 3: Forming a covering QRegion from a set of rectangles, and a
//         reference window
//

struct Intersecter
{
    const QRect m_rect;
    explicit Intersecter(const QRect& rect) : m_rect(rect) {}

    using result_type = QRect;

    QRect operator()(const QRect& rect) const {
        return m_rect.intersected(rect);
    }
};

static void build_region(QRegion& region, const QRect& rect)
{
    if (!rect.isEmpty()) {
        region = region.united(rect);
    }
}

static QRegion coverage(const Container<QRect>& c, const QRect& window)
{
    if (SEQUENTIAL) {
        QRegion result;
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        for (const QRect& r : std::as_const(c)) {
#else
        for (const QRect& r : qAsConst(c)) {
#endif
            const QRect intersected = window.intersected(r);
            if (!intersected.isEmpty()) {
                result = result.united(intersected);
            }
        }
        return result;
    } else {
        // ### Implement here
        return QtConcurrent::blockingMappedReduced(c, Intersecter(window), build_region);
    }
}

//
// Task 4: Forming a QPainterPath from only the polygons which are
//         visible in a reference window
//

struct IntersectTest
{
    const QRectF m_rect;
    explicit IntersectTest(const QRectF& rect) : m_rect(rect) {}

    using result_type = bool;

    bool operator()(const QPolygonF& poly) const {
        return m_rect.intersects(poly.boundingRect());
    }
};

static void build_path(QPainterPath& path, const QPolygonF& poly)
{
    path.addPolygon(poly);
    path.closeSubpath();
}

static QPainterPath visible(const Container<QPolygonF>& c, const QRectF& window)
{
    if (SEQUENTIAL) {
        QPainterPath result;
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        for (const QPolygonF& p : std::as_const(c)) {
#else
        for (const QPolygonF& p : qAsConst(c)) {
#endif
            if (p.boundingRect().intersects(window)) {
                result.addPolygon(p);
                result.closeSubpath();
            }
        }
        return result;
    } else {
        // ### Implement here
        return QtConcurrent::blockingFilteredReduced(c, IntersectTest(window), build_path);
    }
}

//
// Timing Harness
//

static QRandomGenerator* randomGenerator;

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);

    qDebug("QThreadPool::maxThreadCount: %d", QThreadPool::globalInstance()->maxThreadCount());

    randomGenerator = new QRandomGenerator(0U);

    // Timing: exercise each task ITERATIONS times, and output the
    // Time it took (in wallclock seconds)
    QElapsedTimer timer;

    const Container<double> doubles = makeDoubles();

    {
        const Container<QString> strings = makeStrings(doubles);
        QVector<Container<QString>> stringLists;
        for (int i = 0; i < ITERATIONS; ++i) {
            stringLists.push_back(strings);
            stringLists.back().detach();
        }

        timer.start();
        for (int i = 0; i < ITERATIONS; ++i) {
            (void)grepFirstLetterLowerCase(stringLists.at(i));
        }
        qDebug("strings timing (copying): %.3f secs", timer.elapsed() / 1000. / ITERATIONS);

        if (OPTIONALPART) {
            timer.start();
            for (int i = 0; i < ITERATIONS; ++i) {
                grepFirstLetterLowerCaseInline(stringLists[i]);
            }
            qDebug("strings timing (inline): %.3f secs", timer.elapsed() / 1000. / ITERATIONS);
        }
    }

    {
        QVector<Container<double>> doubleLists;
        for (int i = 0; i < ITERATIONS; ++i) {
            doubleLists.push_back(doubles);
            doubleLists.back().detach();
        }

        timer.start();
        for (int i = 0; i < ITERATIONS; ++i) {
            (void)calculateFormula(doubleLists[i]);
        }
        qDebug("doubles timing (copying): %.3f secs", timer.elapsed() / 1000. / ITERATIONS);

        if (OPTIONALPART) {
            timer.start();
            for (int i = 0; i < ITERATIONS; ++i) {
                calculateFormulaInline(doubleLists[i]);
            }
            qDebug("doubles timing (inline): %.3f secs", timer.elapsed() / 1000. / ITERATIONS);
        }
    }

    {
        const Container<QRect> rectangles = makeRectangles(doubles);
        const QRect  reference(QPoint(-500, -500), QPoint(500, 500));
        timer.start();
        for (int i = 0; i < ITERATIONS; ++i) {
            (void)coverage(rectangles, reference);
        }
        qDebug("rectangles timing: %.3f secs", timer.elapsed() / 1000. / ITERATIONS);
    }

    {
        const Container<QPolygonF> polygons = makePolygons(doubles);
        const QRectF referenceF(QPointF(-500, -500), QPointF(500, 500));
        timer.start();
        for (int i = 0; i < ITERATIONS; ++i) {
            (void)visible(polygons, referenceF);
        }
        qDebug("polygons timing: %.3f secs", timer.elapsed() / 1000. / ITERATIONS);
    }
}

//
// Just test data generation below, not really interesting
//

static const int NUM_RANDOM_DOUBLES = 4 * 1024 * 1024;

static Container<double> makeDoubles()
{
    Container<double> result;
    for (int i = 0; i < NUM_RANDOM_DOUBLES; ++i) {
        result.push_back(std::fmod(randomGenerator->generate(), 2000.) + 1000.);
    }
    return result;
}

static Container<QString> makeStrings(const Container<double>& doubles)
{
    Container<QString> result;
    // base64 encode the doubles:
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    for (double d : std::as_const(doubles)) {
#else
    for (double d : qAsConst(doubles)) {
#endif
        const void* dp = &d;
        const QByteArray data(static_cast<const char*>(dp), sizeof(double));
        result.push_back(QString::fromLatin1(data.toBase64()));
    }
    return result;
}

static Container<QRect> makeRectangles(const Container<double>& doubles)
{
    // make rectangles with the random doubles as coordinates:
    Container<QRect> result;
    for (int i = 0; i < doubles.size(); i += 4) {
        result.push_back(QRectF(QPointF(doubles[i+0], doubles[i+1]),
                                QPointF(doubles[i+2], doubles[i+3])).normalized().toRect());
    }
    return result;
}

static Container<QPolygonF> makePolygons(const Container<double>& doubles)
{
    // make polygons with the random doubles as coordinates:
    Container<QPolygonF> result;
    for (int i = 0; i < doubles.size(); i += 16) {
        QPolygonF polygon;
        for (int j = 0; j < 16; j += 2) {
            polygon.push_back(QPointF(doubles[i+j], doubles[i+j+1]));
        }
        result.push_back(polygon);
    }
    return result;
}
