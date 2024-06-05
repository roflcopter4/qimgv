//#define USE_VLD
#if defined _WIN32 && defined _MSC_VER && defined USE_VLD
# include <Shlwapi.h>
# include <F:/Program Files (x86)/Visual Leak Detector/include/vld.h>
#endif

#include <QApplication>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QEvent>

#include "appversion.h"
#include "settings.h"
#include "components/actionmanager/actionmanager.h"
#include "utils/inputmap.h"
#include "utils/actions.h"
#include "utils/cmdoptionsrunner.h"
#include "sharedresources.h"
#include "proxystyle.h"
#include "core.h"

#ifdef Q_OS_APPLE
# include "macosapplication.h"
#endif
#ifdef Q_OS_WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <Windows.h>
#endif

using namespace std::literals;

//------------------------------------------------------------------------------

static void saveSettings()
{
    delete settings;
}

static QDataStream &operator<<(QDataStream &out, Script const &v)
{
    out << v.command << v.blocking;
    return out;
}

static QDataStream &operator>>(QDataStream &in, Script &v)
{
    in >> v.command;
    in >> v.blocking;
    return in;
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    // Force some environment variables.
#ifdef Q_OS_WIN32
# ifdef _DEBUG
    util::OpenConsoleWindow();
# else
    ::FreeConsole();
# endif

# ifdef USE_VLD
    VLDGlobalEnable();
    VLDSetReportOptions(VLD_OPT_UNICODE_REPORT | VLD_OPT_REPORT_TO_FILE, LR"(D:\ass\GIT\qimgv\MSVC_2\vld_report.log)");
    VLDSetOptions(VLD_OPT_TRACE_INTERNAL_FRAMES | VLD_OPT_SKIP_CRTSTARTUP_LEAKS | VLD_OPT_VALIDATE_HEAPFREE | VLD_OPT_AGGREGATE_DUPLICATES | VLD_OPT_SKIP_HEAPFREE_LEAKS, 256, 256);
# endif

    // If this is set by other app, platform plugins may fail to load.
    // https://github.com/easymodo/qimgv/issues/410
    qputenv("QT_PLUGIN_PATH", "");
#endif
#if 0
    // For hidpi testing.
    qputenv("QT_SCALE_FACTOR","1.5");
    qputenv("QT_SCREEN_SCALE_FACTORS", "1;1.7");
#endif

    // Do we still need this?
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0");

    // Some qt5 hidpi vars.
#if (QT_VERSION_MAJOR == 5)
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // Qt6 hidpi rendering on windows still has artifacts.
    // This disables it for scale factors < 1.75 in this case only fonts are scaled.
#ifdef Q_OS_WIN32
# if (QT_VERSION_MAJOR == 6)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
# endif
#endif

#if 0
    qDebug() << qgetenv("QT_SCALE_FACTOR");
    qDebug() << qgetenv("QT_SCREEN_SCALE_FACTORS");
    qDebug() << qgetenv("QT_ENABLE_HIGHDPI_SCALING");
#endif

#ifdef Q_OS_APPLE
    MacOSApplication app(argc, argv);
    // default to "fusion" if available ("macos" has layout bugs, weird comboboxes etc)
    if (QStyleFactory::keys().contains("Fusion"))
        a.setStyle(QStyleFactory::create("Fusion"));
#else
    auto app   = QApplication(argc, argv);
    auto style = new ProxyStyle();
    QApplication::setStyle(style);

    //QApplication app(argc, argv);
    // use some style workarounds
    //QApplication::setStyle(new ProxyStyle);
#endif

    QCoreApplication::setOrganizationName(QS("qimgv"));
    QCoreApplication::setOrganizationDomain(QS("github.com/easymodo/qimgv"));
    QCoreApplication::setApplicationName(QS("qimgv"));
    QCoreApplication::setApplicationVersion(appVersion.toString());
    QApplication::setEffectEnabled(Qt::UI_AnimateCombo, false);
    QGuiApplication::setDesktopFileName(QCoreApplication::applicationName() + QSV(".desktop"));

    // Needed for mpv.
#ifndef _MSC_VER
    setlocale(LC_NUMERIC, "C");
#endif

#ifdef __GLIBC__
    // Default value of 128k causes memory fragmentation issues.
    mallopt(M_MMAP_THRESHOLD, 64000);
#endif

#ifdef USE_EXIV2
# if EXIV2_TEST_VERSION(0, 27, 4)
    Exiv2::enableBMFF(true);
# endif
#endif

    // Use custom types in signals.
    qRegisterMetaType<ScalerRequest>("ScalerRequest");
    qRegisterMetaType<Script>("Script");
    qRegisterMetaType<QSharedPointer<Image>>("QSharedPointer<Image>");
    qRegisterMetaType<QSharedPointer<Thumbnail>>("QSharedPointer<Thumbnail>");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qRegisterMetaTypeStreamOperators<Script>("Script");
#endif

    // Globals
    inputMap      = InputMap::getInstance();
    appActions    = Actions::getInstance();
    settings      = Settings::getInstance();
    scriptManager = ScriptManager::getInstance();
    actionManager = ActionManager::getInstance();
    shrRes        = SharedResources::getInstance();

    atexit(saveSettings);

    // Parse args ------------------------------------------------------------------
    QString appDescription = qApp->applicationName() + QSV(" - Fast and configurable image viewer.") +
                             QSV("\nVersion: ") + qApp->applicationVersion() +
                             QSV("\nLicense: GNU GPLv3");

    QCommandLineParser parser;
    parser.setApplicationDescription(appDescription);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QS("path"), QCoreApplication::translate("main", "File or directory path."));

    parser.addOptions({
        {QS("gen-thumbs"),      QCoreApplication::translate("main", "Generate all thumbnails for directory."),                 QCoreApplication::translate("main", "directory-path")},
        {QS("gen-thumbs-size"), QCoreApplication::translate("main", "Thumbnail size. Current size is used if not specified."), QCoreApplication::translate("main", "thumbnail-size")},
        {QS("build-options"),   QCoreApplication::translate("main", "Show build options.")},
    });

    parser.process(app);

    if (parser.isSet(QS("build-options"))) {
        CmdOptionsRunner r;
        QTimer::singleShot(0, &r, &CmdOptionsRunner::showBuildOptions);
        return QApplication::exec();
    }
    if (parser.isSet(QS("gen-thumbs"))) {
        int size = parser.isSet(QS("gen-thumbs-size"))
                       ? parser.value(QS("gen-thumbs-size")).toInt()
                       : settings->folderViewIconSize();

        //CmdOptionsRunner r;
        //QTimer::singleShot(0, &r, std::bind(&CmdOptionsRunner::generateThumbs, &r, parser.value(QS("gen-thumbs")), size));
        QTimer::singleShot(0,
            [&parser, size] {
                CmdOptionsRunner::generateThumbs(parser.value(QS("gen-thumbs")), size);
            });

        return QApplication::exec();
    }

    // -----------------------------------------------------------------------------

    auto core = new Core(nullptr);

#ifdef Q_OS_APPLE
    QObject::connect(&app, &MacOSApplication::fileOpened, &core, &Core::loadPath);
#endif

    if (parser.positionalArguments().count())
        core->loadPath(parser.positionalArguments()[0]);
    else if (settings->defaultViewMode() == ViewMode::FOLDERVIEW)
        core->loadPath(QDir::homePath());

    // Wait for event queue to catch up before showing window.
    // This avoids white background flicker on windows (or not?).
    qApp->processEvents();

#if 0
    QThread *foo = QThread::create([]() {
        qDebug() << u"Hello, world!";
        QThread::sleep(1s);
        QThread::currentThread()->exit(0);
    });
    foo->start(QThread::Priority::HighPriority);
    auto r = foo->wait(30s);
    qDebug() << u"Exit" << r;
    delete foo;
#endif

#ifdef USE_VLD
    VLDRefreshModules();
#endif

    core->showGui();
    int ret = QApplication::exec();

#ifdef USE_VLD
    VLDReportLeaks();
    qDebug() << u"Leaks:" << VLDGetLeaksCount();
    util::WaitForAnyKey();
    VLDSetReportOptions(VLD_OPT_UNICODE_REPORT | VLD_OPT_REPORT_TO_FILE, LR"(D:\ass\GIT\qimgv\MSVC_2\vld_report_final.log)");
#endif

    //util::DeleteAndNullify(inputMap);
    //util::DeleteAndNullify(appActions);
    //util::DeleteAndNullify(scriptManager);
    //util::DeleteAndNullify(actionManager);
    //util::DeleteAndNullify(shrRes);
    //util::DeleteAndNullify(settings);
    //util::DeleteAndNullify(style);

    delete core;
    return ret;
}
