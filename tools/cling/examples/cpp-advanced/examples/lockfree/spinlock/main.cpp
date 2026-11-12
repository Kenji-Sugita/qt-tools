#include <QCoreApplication>
#include <QThread>
#include <QAtomicInteger>
#include <QMutex>
#include <QDebug>

#define USE_SPINLOCK
#define USE_FETCHANDSTORE
//#define USE_STD_ATOMIC

#if !defined(USE_SPINLOCK)
//#define USE_STD_MUTEX
#endif

#ifdef USE_SPINLOCK
#if defined(USE_STD_ATOMIC)
static std::atomic<qint32> atomicInteger;
#else
static QAtomicInteger<qint32> atomicInteger;
#endif
#else // USE_SPINLOCK
#if defined(USE_STD_MUTEX)
std::mutex mutex;
#else
QMutex mutex;
#endif
#endif // USE_SPINLOCK

static int nonAtomicInteger;

#ifdef USE_SPINLOCK
#ifdef USE_FETCHANDSTORE
void spinLock()
{
#if defined(USE_STD_ATOMIC)
    while (atomicInteger.exchange(1, std::memory_order_acquire) == 1) {
#else
    while (atomicInteger.fetchAndStoreAcquire(1)) {
#endif
        ;
    }
}

void spinUnlock()
{
#if defined(USE_STD_ATOMIC)
    atomicInteger.store(0, std::memory_order_release);
#else
    atomicInteger.fetchAndStoreRelease(0);
#endif
}
#else // USE_FETCHANDSTORE
void spinLock()
{
#if defined(USE_STD_ATOMIC)
    qint32 expected = 0;
    while (!atomicInteger.compare_exchange_weak(expected, 1, std::memory_order_acquire) && expected == 0) {
#else
    while (!atomicInteger.testAndSetAcquire(0, 1)) {
#endif
        ;
    }
}

void spinUnlock()
{
#if defined(USE_STD_ATOMIC)
    qint32 expected = 1;
    if (!atomicInteger.compare_exchange_weak(expected, 0, std::memory_order_release) && expected == 1) {
#else
    if (!atomicInteger.testAndSetRelease(1, 0)) {
#endif
	qDebug() << "Unlock: Not locked.";
    }
}
#endif // USE_FETCHANDSTORE
#endif // USE_SPINLOCK

class ValueChangeThread : public QThread
{
    Q_OBJECT

public:
    explicit ValueChangeThread(int value, QObject* parent = nullptr)
        : QThread(parent), value(value) {
    }

protected:
    void run() override {
        for (int n = 0; n < 1000*1000; n++) {
#ifdef USE_SPINLOCK
            spinLock();    // Acquire fence: The following instructions don't move before this line
            nonAtomicInteger += value;
            spinUnlock();  // Release fence: The preceding instructions don't move after this line
#else
            mutex.lock();    // Mutex ensures a memory fence by its specification
            nonAtomicInteger += value;
            mutex.unlock();  // Mutex ensures a memory fence by its specification
#endif
        }
    }

private:
    const int value;
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

#ifdef USE_SPINLOCK
    qDebug() << "atomicInteger =" << int(atomicInteger);
#endif
    nonAtomicInteger = 0;

    qDebug() << "nonAtomicInteger = " << nonAtomicInteger;

    ValueChangeThread incrementThread(1);
    ValueChangeThread decrementThread(-1);
    const int maxCount = 10;
    for (int count = 0; count < maxCount; count++ ) {
        qDebug() << count;
        incrementThread.start();
        decrementThread.start();
        incrementThread.wait();
        decrementThread.wait();
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    qDebug() << Qt::endl << "nonAtomicInteger = " << nonAtomicInteger;
#else
    qDebug() << endl << "nonAtomicInteger = " << nonAtomicInteger;
#endif
}

#include "main.moc"
