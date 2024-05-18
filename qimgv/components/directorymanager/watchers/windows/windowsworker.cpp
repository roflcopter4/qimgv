#include "Common.h"
#include "windowsworker.h"

#include <QMessageBox>


void WindowsWorker::setDirectoryHandle(HANDLE handle)
{
    // qDebug() << "setHandle" << this->handle << " -> " << handle;
    freeHandle();
    hDir = handle;
}

void WindowsWorker::freeHandle()
{
    if (hDir == INVALID_HANDLE_VALUE)
        return;
    CancelIoEx(hDir, nullptr);
    CloseHandle(hDir);
    hDir = INVALID_HANDLE_VALUE;
}


void WindowsWorker::run()
{
    using namespace std::literals;

    //if (isRunning.exchange(true, std::memory_order::seq_cst))
    //    return;

    DWORD      dwBytes = 0;
    OVERLAPPED ovl = {.hEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr)};

    if (!ovl.hEvent || ovl.hEvent == INVALID_HANDLE_VALUE) {
        qDebug() << QSV(u"[WindowsWorker] CreateEvent failed?");
        QMessageBox::warning(
            nullptr, QS("Error"),
            QS("CreateEvent failed in " __FILE__ ". \n"
               "This probably means everything will break catastrophically. Fairly warned, be thee, says I.")
        );
    }

    static constexpr size_t BUFFER_SIZE = 1024ULL * 64ULL;
    auto buffer = std::make_unique<BYTE[]>(BUFFER_SIZE);

    while (isRunning.load(std::memory_order::relaxed))
    {
        bool bPending = ReadDirectoryChangesW(
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
            DWORD error = GetLastError();
            if (error == ERROR_IO_INCOMPLETE)
                qDebug() << QSV(u"ERROR_IO_INCOMPLETE");
        }
        else if (::GetOverlappedResult(hDir, &ovl, &dwBytes, false)) {
            if(dwBytes != 0) {
                auto *fni = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(buffer.get());
                do {
                    if(fni->Action != 0) {
                        emit notifyEvent(fni);
                    }
                    if(fni->NextEntryOffset == 0)
                        break;
                    fni = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(reinterpret_cast<PCHAR>(fni) + fni->NextEntryOffset);
                } while (true);
            }
        }

        ::Sleep(POLL_RATE_MS);
    }
}
