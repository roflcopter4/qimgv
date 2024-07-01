#include "WindowsDirectoryWorker.h"
#include <QMessageBox>
#include <QThread>


WindowsDirectoryWorker::WindowsDirectoryWorker()
{
    ::InitializeCriticalSection(&criticalSection);
}

WindowsDirectoryWorker::~WindowsDirectoryWorker()
{
    ::DeleteCriticalSection(&criticalSection);
}

void WindowsDirectoryWorker::setDirectoryHandle(HANDLE handle)
{
    ::EnterCriticalSection(&criticalSection);
    if (hDir != INVALID_HANDLE_VALUE) {
        ::CancelIoEx(hDir, nullptr);
        ::CloseHandle(hDir);
    }
    hDir = handle;
    ::LeaveCriticalSection(&criticalSection);
}

void WindowsDirectoryWorker::iterateDirectoryEvents(PCBYTE buffer)
{
    auto fni = reinterpret_cast<PCFILE_NOTIFY_INFORMATION>(buffer);
    for (;;) {
        if (fni->Action != 0) {
            size_t size  = sizeof(FILE_NOTIFY_INFORMATION) + fni->FileNameLength * sizeof(wchar_t);
            auto   event = std::make_unique<BYTE[]>(size);
            memcpy(event.get(), fni, size);
            Q_EMIT notifyEvent(event.release());
        }
        if (fni->NextEntryOffset == 0)
            break;
        fni = reinterpret_cast<PCFILE_NOTIFY_INFORMATION>(reinterpret_cast<PCBYTE>(fni) + fni->NextEntryOffset);
    }
}

void WindowsDirectoryWorker::run()
{
    DWORD      dwBytes = 0;
    OVERLAPPED ovl = {.hEvent = ::CreateEventW(nullptr, false, false, L"ReadDirectoryChangesW Overlapped Event")};

    if (!ovl.hEvent || ovl.hEvent == INVALID_HANDLE_VALUE) {
        qDebug() << u"[WindowsWorker] CreateEvent failed?";
        QMessageBox::warning(
            nullptr, u"Error"_s,
            u"CreateEvent failed in " + QString::fromLatin1(PRETTY_FUNCTION_SIG) + u".\n"
            u"This probably means everything will break catastrophically. Fairly warned, be thee, says I."
        );
    }

    auto buffer = std::make_unique<BYTE[]>(BUFFER_SIZE);

    while (isRunning.load(std::memory_order::acquire))
    {
        bool bPending = ::ReadDirectoryChangesW(
            hDir,
            buffer.get(),
            BUFFER_SIZE,
            FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &dwBytes,
            &ovl,
            nullptr
        );
        if (!isRunning.load(std::memory_order::seq_cst))
            break;

        DWORD error;
        if (bPending) {
            if (::GetOverlappedResult(hDir, &ovl, &dwBytes, false)) {
                error = ::GetLastError();
                if (dwBytes != 0)
                    iterateDirectoryEvents(buffer.get());
                if (error == ERROR_IO_INCOMPLETE || error == ERROR_NOTIFY_ENUM_DIR)
                    goto BadIO;
            }
        } else {
            error = ::GetLastError();
            if (error == ERROR_IO_INCOMPLETE || error == ERROR_NOTIFY_ENUM_DIR) {
            BadIO:
                ::MessageBoxW(nullptr,
                    L"An asynchronous IO operation in WindowDirectoryWorker has not completed due to insufficient buffer size. "
                    L"The event will be ignored and files listed in the current folder will be out of date. You can re-synchronize "
                    L"it by reloading the folder entirely.",
                    L"Error", MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND
                );
            }
        }

        ::Sleep(POLL_RATE_MS);
    }

    ::EnterCriticalSection(&criticalSection);
    if (hDir && hDir != INVALID_HANDLE_VALUE) {
        ::CloseHandle(hDir);
        hDir = INVALID_HANDLE_VALUE;
    }
    ::LeaveCriticalSection(&criticalSection);
    QThread::currentThread()->exit();
}
