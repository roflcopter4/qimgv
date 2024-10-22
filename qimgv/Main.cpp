#include <QApplication>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QEvent>

#include "AppVersion.h"
#include "Settings.h"
#include "components/actionManager/ActionManager.h"
#include "utils/InputMap.h"
#include "utils/Actions.h"
#include "utils/CmdOptionsRunner.h"
#include "SharedResources.h"
#include "ProxyStyle.h"
#include "Core.h"

//#define USE_VLD

#if defined Q_OS_APPLE
# include "macosapplication.h"
#elif defined Q_OS_WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <Windows.h>
# if !defined USE_VLD && defined _MSC_VER && __has_include(<vld.h>) && false
#  define USE_VLD
# endif
# ifdef USE_VLD
#  define VLD_FORCE_ENABLE 1
#  include <Shlwapi.h>
#  include <vld.h>
# endif
#endif
#define tr(...) QApplication::translate(__VA_ARGS__)

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
# if defined _DEBUG
    util::OpenConsoleWindow();
# else
    ::FreeConsole();
# endif

# ifdef USE_VLD
    ::VLDGlobalEnable();
    ::VLDSetReportOptions(0, nullptr);
    ::VLDSetOptions(VLD_OPT_AGGREGATE_DUPLICATES | VLD_OPT_SAFE_STACK_WALK, 256, 128);
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
#if defined Q_OS_WIN32 && QT_VERSION_MAJOR == 6
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
#endif

#ifdef Q_OS_APPLE
    auto *app = new MacOSApplication (argc, argv);
    // default to "fusion" if available ("macos" has layout bugs, weird comboboxes etc)
    if (QStyleFactory::keys().contains("Fusion"))
        app->setStyle(QStyleFactory::create("Fusion"));
#else
    auto *app   = new QApplication(argc, argv);
    auto *style = new ProxyStyle(nullptr);
    QApplication::setStyle(style);
#endif

    QCoreApplication::setOrganizationName(u"qimgv"_s);
    QCoreApplication::setOrganizationDomain(u"github.com/easymodo/qimgv"_s);
    QCoreApplication::setApplicationName(u"qimgv"_s);
    QCoreApplication::setApplicationVersion(appVersion.toString());
    QGuiApplication::setDesktopFileName(QCoreApplication::applicationName() + u".desktop");
    QApplication::setEffectEnabled(Qt::UI_AnimateCombo, false);

    // Needed for mpv.
#ifndef _MSC_VER
    setlocale(LC_NUMERIC, "C");
#endif
#ifdef __GLIBC__
    // Default value of 128k causes memory fragmentation issues.
    mallopt(M_MMAP_THRESHOLD, 65536);
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
    appActions    = Actions::getInstance();
    settings      = Settings::getInstance();
    scriptManager = ScriptManager::getInstance();
    actionManager = ActionManager::getInstance();
    shrRes        = SharedResources::getInstance();

    ::atexit(saveSettings);

    // Parse args ------------------------------------------------------------------
    QString appDescription = qApp->applicationName() + u" - Fast and configurable image viewer.\n"
                             u"Version: " + qApp->applicationVersion() + u"\n"
                             u"License: GNU GPLv3";

    auto *parser = new QCommandLineParser;
    parser->setApplicationDescription(appDescription);
    parser->addHelpOption();
    parser->addVersionOption();
    parser->addPositionalArgument(u"path"_s, tr("main", "File or directory path."));
    parser->addOptions({
        {u"gen-thumbs"_s,      tr("main", "Generate all thumbnails for directory."),                 tr("main", "directory-path")},
        {u"gen-thumbs-size"_s, tr("main", "Thumbnail size. Current size is used if not specified."), tr("main", "thumbnail-size")},
        {u"build-options"_s,   tr("main", "Show build options.")},
    });
    parser->process(*app);

    if (parser->isSet(u"build-options"_s)) {
        CmdOptionsRunner r;
        QTimer::singleShot(0, &r, &CmdOptionsRunner::showBuildOptions);
        return QApplication::exec();
    }
    if (parser->isSet(u"gen-thumbs"_s)) {
        int size = parser->isSet(u"gen-thumbs-size"_s)
                       ? parser->value(u"gen-thumbs-size"_s).toInt()
                       : settings->folderViewIconSize();

        //CmdOptionsRunner r;
        //QTimer::singleShot(0, &r, std::bind(&CmdOptionsRunner::generateThumbs, &r, parser.value(u"gen-thumbs"_s), size));
        QTimer::singleShot(
            0, [parser, size] {
                CmdOptionsRunner::generateThumbs(parser->value(u"gen-thumbs"_s), size);
            }
        );

        return QApplication::exec();
    }
    // -----------------------------------------------------------------------------

    auto *core = new Core(nullptr);
#ifdef Q_OS_APPLE
    QObject::connect(&app, &MacOSApplication::fileOpened, &core, &Core::loadPath);
#endif
    if (parser->positionalArguments().count())
        core->loadPath(parser->positionalArguments()[0]);
    else if (settings->defaultViewMode() == ViewMode::FOLDERVIEW)
        core->loadPath(QDir::homePath());

    // Wait for event queue to catch up before showing window.
    // This avoids white background flicker on windows (or not?).
    qApp->processEvents();

    // Run the show.
    core->showGui();
    int ret = QApplication::exec();

    util::DeleteAndAssignNull(parser, core, appActions, scriptManager, actionManager, shrRes, settings, app);

#ifdef USE_VLD
    ::VLDSetReportOptions(VLD_OPT_UNICODE_REPORT | VLD_OPT_REPORT_TO_FILE,
                          (LR"(F:\Ass\GIT\qimgv\MSVC_2\#LOGS\vld_report_)" + std::to_wstring(::_time64(nullptr)) + L".log").c_str());
    ::fwprintf(stderr, L"Leaks: %u\n", ::VLDGetLeaksCount());
# ifdef _DEBUG
    util::WaitForAnyKey();
# endif
#endif

    return ret;
}
