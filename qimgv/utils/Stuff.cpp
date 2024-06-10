#include "Stuff.h"
#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <atomic>
#include <iostream>
#include <mutex>
#include <qcoreapplication.h>

#ifdef Q_OS_WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <Windows.h>
# include <DbgHelp.h>
#elif defined __has_include && __has_include(<execinfo.h>)
# include <execinfo.h>
#endif
#if defined __GNUG__ || defined _LIBCPP_VERSION
# include <cxxabi.h>
#endif

using namespace std::literals;

/****************************************************************************************/

int clamp(int x, int lower, int upper)
{
    return qMin(upper, qMax(x, lower));
}

// 0 - mac, 1 - linux, 2 - windows, 3 - other
int probeOS()
{
#ifdef TARGET_OS_MAC
    return 0;
#elif defined Q_OS_LINUX
    return 1;
#elif defined Q_OS_WINDOWS
    return 2;
#else
    return 3;
#endif
}

namespace util {

StdString QStringToStdString(QString const &str)
{
#ifdef Q_OS_WIN32
    return str.toStdWString();
#else
    return str.toStdString();
#endif
}

QString StdStringToQString(StdString const &str)
{
#ifdef Q_OS_WIN32
    return QString::fromStdWString(str);
#else
    return QString::fromStdString(str);
#endif
}

QString StdPathToQString(std::filesystem::path const &str)
{
    return StdStringToQString(str.native());
}

std::filesystem::path
QStringToStdPath(QString const &filePath)
{
#ifdef Q_OS_WIN32
    if (filePath.startsWith(QSV(R"(\\?\)")))
        return {std::basic_string_view{filePath.data_ptr().data(),
                                       static_cast<size_t>(filePath.size())}};
    auto tmp = QFileInfo(filePath).absoluteFilePath();
    tmp.replace(u'/', u'\\');
    tmp.prepend(QSV(R"(\\?\)"));
    return {std::basic_string_view{tmp.data_ptr().data(),
                                   static_cast<size_t>(tmp.size())}};
#else
    return absolute(std::filesystem::path{filePath.toStdString()});
#endif
}

/****************************************************************************************/


static void demangle_setup()
{
#if defined _MSC_VER
    static std::mutex       mtx;
    static std::atomic_flag flg;

    std::lock_guard lock(mtx);
    if (flg.test_and_set())
        return;
    //if (::IsDebuggerPresent())
    //    return;
    ::SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    if (::BOOL ret = ::SymInitialize(::GetCurrentProcess(), nullptr, true); !ret) {
        ::DWORD e = ::GetLastError();
        qCritical() << GetErrorMessage(e);
        qFatal() << u"SymInitialize()";
    }
#endif
}

[[maybe_unused]] static std::string demangle(char const *raw_name)
{
    demangle_setup();
    char const *name;

#if defined _MSC_VER
    char buf[4096];
    ::UnDecorateSymbolName(raw_name, buf, static_cast<DWORD>(std::size(buf)), UNDNAME_COMPLETE);
    name = buf;
#elif defined __GNUG__ || defined _LIBCPP_VERSION
    int  status;
    auto demangled = std::unique_ptr<char, void (*)(void *)>{
        abi::__cxa_demangle(raw_name, nullptr, nullptr, &status), ::free};
    if (status != 0) {
        demangled.reset();
        demangled = std::unique_ptr<char, void (*)(void *)>{
            abi::__cxa_demangle(("_"s + raw_name).c_str(), nullptr, nullptr, &status), ::free};
        if (status != 0) {
            name = raw_name;
        } else {
            name = demangled.get();
        }
    } else {
        name = demangled.get();
    }
#else
    name = raw_name;
#endif

    return std::string{name};
}

[[maybe_unused]] static std::wstring demangle(wchar_t const *raw_name)
{
    demangle_setup();
    wchar_t const *name;

#if defined _WIN32 && defined _MSC_VER
    wchar_t buf[4096];
    ::UnDecorateSymbolNameW(raw_name, buf, static_cast<DWORD>(std::size(buf)), UNDNAME_COMPLETE);
    name = buf;
#else
    name = raw_name;
#endif

    return std::wstring{name};
}

/****************************************************************************************/

#if 0

QString GetBacktrace()
{
    // g_on_error_stack_trace(program_invocation_short_name);
    std::stringstream ss;
    ss << boost::stacktrace::stacktrace();
    buf.push_back('\n');
    buf.append(ss.str());
    return buf;
}

#elif __has_include(<execinfo.h>)

QString GetBacktrace()
{
    void   *arr[256];
    int     num     = ::backtrace(arr, 256);
    char  **symbols = ::backtrace_symbols(const_cast<void *const *>(arr), num);
    QString buf     = QS("\n\n\033[1mBACKTRACE:\033[0m");

    for (int i = 0; i < num; ++i) {
        char *ptr = ::strchr(symbols[i], '(');
        if (!ptr || !*++ptr)
            continue;

        ptrdiff_t len = ptr - symbols[i];

        if (ptr[0] == '_' && ptr[1] == 'Z') {
            buf += std::format("\n{:3}: {}", i, std::string_view(symbols[i], len));

            char *end = ::strchr(ptr, '+');
            if (end) {
                bool success = true;
                *end         = '\0';
                try {
                    buf += std::format("\033[0;36m{}\033[0m + ", demangle(ptr));
                } catch (...) {
                    success = false;
                }
                if (success)
                    ptr = end + 1;
                else
                    *end = '+';
            }

            buf += QString::fromUtf8(ptr);
        } else if (ptr[0] != '+') {
            char *end = ::strchr(ptr, '+');
            if (end) {
                *end = '\0';
                buf += std::format("\n{:3}: {}\033[0;36m{}\033[0m + ",
                                   i, std::string_view(symbols[i], len), std::string_view(ptr, end - ptr));
                ptr = end + 1;
            }
            buf += QString::fromUtf8(ptr);
        } else {
            buf += std::format("\n{:3}: {}", i, symbols[i]);
        }
    }

    // NOLINTNEXTLINE(hicpp-no-malloc, cppcoreguidelines-no-malloc)
    free(symbols);
    return buf;
}

#elif defined Q_OS_WIN32

QString GetBacktrace()
{
# ifdef _MSC_VER
    static constexpr size_t  namelen  = 1024ULL * sizeof(wchar_t);
    static constexpr wchar_t format[] = L"\n{:3}: {}  -  [0x{:08X}]";
    using symbol_type = ::SYMBOL_INFOW;
#  define SymFromAddr SymFromAddrW
# else
    static constexpr size_t namelen = 1024ULL * sizeof(char);
    using symbol_type = ::SYMBOL_INFO;
#  define SymFromAddr SymFromAddr
# endif

    void    *stack[256];
    ::HANDLE heap    = ::GetProcessHeap();
    ::HANDLE process = ::GetCurrentProcess();
    ::SymInitializeW(process, nullptr, true);

    ::WORD frames = ::RtlCaptureStackBackTrace(0UL, 256UL, stack, nullptr);
    auto  *symbol = static_cast<symbol_type *>(::HeapAlloc(heap, HEAP_ZERO_MEMORY, sizeof(symbol_type) + namelen));
    if (!symbol)
        return {};

    symbol->MaxNameLen   = (namelen - 1) / sizeof(char);
    symbol->SizeOfStruct = sizeof(symbol_type);
    QString ret          = QS("\n\n\033[1mBACKTRACE:\033[0m");

    for (unsigned i = 0; i < frames && i < std::size(stack); ++i) {
        ::SymFromAddr(process, reinterpret_cast<::DWORD64>(stack[i]), nullptr, symbol);
# if defined _MSC_VER
        ret += std::format(format, frames-i-1, demangle(symbol->Name), symbol->Address);
        ::HeapFree(heap, 0, symbol);
# else
        ret += QS("\n") + QString::fromStdString(demangle(symbol->Name));
# endif
    }

    return ret;
#undef SymFromAddr
}

#else

QString GetBacktrace() { return QS("Backtrace not supported"); }

#endif

QString GetErrorMessage(unsigned errVal)
{
#if defined Q_OS_WIN
    wchar_t buf[512];
    DWORD res = ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                                 errVal, LANG_SYSTEM_DEFAULT, buf, std::size(buf), nullptr);
    return (res == 0 ? QString::fromWCharArray(buf, res) : QS(u"Unknown error")) +
           QSV(" (") + QString::number(errVal, 16) + u')';
#else
    char buf[512];
    strerror_r(errno, buf, std::size(buf));
    return QString::fromUtf8(buf) + QSV(" (") + QString::number(errVal) + u')';
#endif
}

#ifdef Q_OS_WIN32

static std::atomic_bool console_opened;
static std::mutex       console_mutex;

static void do_OpenConsoleWindow()
{
    ::FreeConsole();
    if (!::AllocConsole()) {
        ::DWORD err = ::GetLastError();
        ::WCHAR buf[128];
        ::swprintf_s(buf, std::size(buf),
                     L"Failed to allocate a console (error %ls). "
                     L"If this happens your computer is probably on fire.",
                     reinterpret_cast<wchar_t const *>(GetErrorMessage(err).data()));
        ::MessageBoxW(nullptr, buf, L"Fatal Error", MB_OK | MB_ICONERROR);
#ifdef QT_VERSION
        ::QCoreApplication::exit(1);
#else
        ::exit(1);
#endif
    }

    int ret;
    ::FILE *conout;
    ret = ::_wfreopen_s(&conout, L"CONOUT$", L"wt", stdout);
    assert(ret == 0);
    ret = ::_wfreopen_s(&conout, L"CONOUT$", L"wt", stderr);
    assert(ret == 0);
    ret = ::_wfreopen_s(&conout, L"CONIN$", L"rt", stdin);
    assert(ret == 0);
}


void OpenConsoleWindow()
{
    std::lock_guard lock(console_mutex);
    if (!console_opened.exchange(true, std::memory_order::relaxed))
        do_OpenConsoleWindow();
}

void CloseConsoleWindow()
{
    std::lock_guard lock(console_mutex);
    if (console_opened.exchange(false, std::memory_order::relaxed))
        ::FreeConsole();
}

void WaitForAnyKey()
{
    std::cout.flush();
    std::wcout << L"\nPress any key (may need to press enter)...";
    std::wcout.flush();
    std::wstring in;
    std::getline(std::wcin, in);
}

#endif

} // namespace util
