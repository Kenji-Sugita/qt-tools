#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <Windows.h>
#include <cstdio>
#include <atomic>

using SetPeriodicCallback = void (*)(void (*)(), int);
using ClearPeriodicCallback = void (*)();

namespace {
int applicationArgumentCount = 1;
char applicationName[] = "cling";
char* applicationArgumentValues[] = { applicationName, nullptr };
QApplication* applicationInstance = nullptr;
ClearPeriodicCallback clearPeriodicCallback = nullptr;
QtMessageHandler previousMessageHandler = nullptr;
bool messageHandlerInstalled = false;
std::atomic<unsigned long long> callbackCount{0};

void writeQtMessage(QtMsgType type, const QMessageLogContext& context,
                    const QString& message)
{
    const QByteArray formatted = qFormatLogMessage(type, context, message)
                                     .toUtf8();
    std::fwrite(formatted.constData(), 1,
                static_cast<size_t>(formatted.size()), stderr);
    std::fputc('\n', stderr);
    std::fflush(stderr);
}
}

QApplication* ensureApplication()
{
    if (QCoreApplication::instance() != nullptr) {
        return static_cast<QApplication*>(QCoreApplication::instance());
    }

    if (applicationInstance == nullptr) {
        applicationInstance =
            new QApplication(applicationArgumentCount, applicationArgumentValues);
    }

    return applicationInstance;
}

void processQtEventsPeriodically()
{
    callbackCount.fetch_add(1, std::memory_order_relaxed);
    if (QCoreApplication::instance() == nullptr) {
        return;
    }

    QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
}

extern "C" __declspec(dllexport) unsigned long long qtcling_callback_count()
{
    return callbackCount.load(std::memory_order_relaxed);
}

extern "C" __declspec(dllexport) void qtcling_initialize()
{
    ensureApplication();
    if (!messageHandlerInstalled) {
        previousMessageHandler = qInstallMessageHandler(&writeQtMessage);
        messageHandlerInstalled = true;
    }
    HMODULE executable = GetModuleHandleW(nullptr);
    auto setPeriodicCallback = reinterpret_cast<SetPeriodicCallback>(
        GetProcAddress(executable, "cling_set_periodic_callback"));
    clearPeriodicCallback = reinterpret_cast<ClearPeriodicCallback>(
        GetProcAddress(executable, "cling_clear_periodic_callback"));
    if (setPeriodicCallback != nullptr) {
        setPeriodicCallback(&processQtEventsPeriodically, 50);
    }
}

extern "C" __declspec(dllexport) void qtcling_shutdown()
{
    if (clearPeriodicCallback != nullptr) {
        clearPeriodicCallback();
    }
    if (messageHandlerInstalled) {
        qInstallMessageHandler(previousMessageHandler);
        previousMessageHandler = nullptr;
        messageHandlerInstalled = false;
    }
}
