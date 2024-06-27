#include "WindowsDirectoryWorker.h"
#include <QMessageBox>
#include <QThread>


void WindowsDirectoryWorker::setDirectoryHandle(HANDLE handle)
{
    freeHandle();
    hDir = handle;
}

void WindowsDirectoryWorker::freeHandle()
{
    if (!hDir || hDir == INVALID_HANDLE_VALUE)
        return;
    HANDLE tmp = hDir;
    hDir       = INVALID_HANDLE_VALUE;
    ::CancelIoEx(tmp, nullptr);
    ::CloseHandle(tmp);
}


void WindowsDirectoryWorker::iterateDirectoryEvents(PCBYTE buffer)
{
    auto fni = reinterpret_cast<PCFILE_NOTIFY_INFORMATION>(buffer);
    for (;;) {
        if (fni->Action != 0) {
            size_t size  = sizeof(FILE_NOTIFY_INFORMATION) + fni->FileNameLength * sizeof(wchar_t);
            auto   event = std::make_unique<BYTE[]>(size);
            memcpy(event.get(), fni, size);
            emit notifyEvent(event.release());
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

        if (!bPending) {
            DWORD error = ::GetLastError();
            if (error == ERROR_IO_INCOMPLETE)
                qDebug() << u"ERROR_IO_INCOMPLETE";
        }
        else if (::GetOverlappedResult(hDir, &ovl, &dwBytes, false)) {
            if (dwBytes != 0)
                iterateDirectoryEvents(buffer.get());
        }

        ::Sleep(POLL_RATE_MS);
    }

    if (hDir && hDir != INVALID_HANDLE_VALUE) {
        ::CloseHandle(hDir);
        hDir = INVALID_HANDLE_VALUE;
    }
    QThread::currentThread()->exit();
}
