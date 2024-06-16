#include "Settings.h"

Settings *settings = nullptr;

Settings::Settings(QObject *parent)
    : QObject(parent),
#ifdef Q_OS_LINUX
      stateConf(QCoreApplication::organizationName(), u"savedState"_s),
      themeConf(QCoreApplication::organizationName(), u"theme"_s)
#else
      mConfDir(QApplication::applicationDirPath() + u"/conf"),
      settingsConf(mConfDir.absolutePath() + u'/' + qApp->applicationName() + u".ini", QSettings::IniFormat),
      stateConf   (mConfDir.absolutePath() + u"/savedState.ini", QSettings::IniFormat),
      themeConf   (mConfDir.absolutePath() + u"/theme.ini", QSettings::IniFormat)
#endif
{
    // config files
#ifdef Q_OS_LINUX
      qDebug() << stateConf.fileName();
      qDebug() << themeConf.fileName();
#else
    mConfDir.mkpath(QApplication::applicationDirPath() + u"/conf");
#endif

    fillVideoFormats();
}

Settings::~Settings()
{
    saveTheme();
}

//------------------------------------------------------------------------------

Settings *Settings::getInstance()
{
    static std::mutex mtx;
    std::lock_guard lock(mtx);

    if (!settings) {
        settings = new Settings();
        settings->setupCache();
        settings->loadTheme();
    }
    return settings;
}

void Settings::setupCache()
{
#ifdef Q_OS_LINUX
    QString genericCacheLocation = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    if (genericCacheLocation.isEmpty())
        genericCacheLocation = QDir::homePath() + u"/.cache"_s;
    genericCacheLocation.append(u"/"_s + QApplication::applicationName());
    mTmpDir = QDir(settingsConf.value(u"cacheDir", genericCacheLocation).toString());
    mTmpDir.mkpath(mTmpDir.absolutePath());

    QFileInfo dirTest(mTmpDir.absolutePath());
    if (!dirTest.isDir() || !dirTest.isWritable() || !dirTest.exists()) {
        // fallback
        qDebug() << u"Error: cache dir is not writable" << mTmpDir.absolutePath();
        qDebug() << u"Trying to use" << genericCacheLocation << u"instead";
        mTmpDir.setPath(genericCacheLocation);
        mTmpDir.mkpath(mTmpDir.absolutePath());
    }
    mThumbCacheDir = QDir(mTmpDir.absolutePath() + u"/thumbnails"_s);
    mThumbCacheDir.mkpath(mThumbCacheDir.absolutePath());
#else
    mTmpDir = QDir(QApplication::applicationDirPath() + u"/cache");
    mTmpDir.mkpath(mTmpDir.absolutePath());
    mThumbCacheDir = QDir(QApplication::applicationDirPath() + u"/thumbnails");
    mThumbCacheDir.mkpath(mThumbCacheDir.absolutePath());
#endif
}

//------------------------------------------------------------------------------

void Settings::sync()
{
    settingsConf.sync();
    stateConf.sync();
}

QString Settings::thumbnailCacheDir() const
{
    return mThumbCacheDir.path() + u'/';
}

QString Settings::tmpDir() const
{
    return mTmpDir.path() + u'/';
}

//------------------------------------------------------------------------------

// This here is temporarily, will be moved to some sort of theme manager class.
void Settings::loadStylesheet()
{
    // stylesheet template file
    QFile file(u":/res/styles/style-template.qss"_s);
    if (!file.open(QFile::ReadOnly))
        return;
    QString styleSheet = QLatin1String(file.readAll());

    // --- Color Scheme ---------------------------------------------
    auto colors = colorScheme();
    // tint color for system windows
    QPalette p;
    QColor   sys_text   = p.text().color();
    QColor   sys_window = p.window().color();
    QColor   sys_window_tinted, sys_window_tinted_lc, sys_window_tinted_lc2, sys_window_tinted_hc, sys_window_tinted_hc2;
    if (sys_window.valueF() <= 0.45f) {
        // dark system theme
        sys_window_tinted_lc2.setHsv(sys_window.hue(), sys_window.saturation(), sys_window.value() + 6);
        sys_window_tinted_lc.setHsv(sys_window.hue(), sys_window.saturation(), sys_window.value() + 14);
        sys_window_tinted.setHsv(sys_window.hue(), sys_window.saturation(), sys_window.value() + 20);
        sys_window_tinted_hc.setHsv(sys_window.hue(), sys_window.saturation(), sys_window.value() + 35);
        sys_window_tinted_hc2.setHsv(sys_window.hue(), sys_window.saturation(), sys_window.value() + 50);
    } else {
        // light system theme
        sys_window_tinted_lc2.setHsv(sys_window.hue(), sys_window.saturation(), sys_window.value() - 6);
        sys_window_tinted_lc.setHsv(sys_window.hue(), sys_window.saturation(), sys_window.value() - 14);
        sys_window_tinted.setHsv(sys_window.hue(), sys_window.saturation(), sys_window.value() - 20);
        sys_window_tinted_hc.setHsv(sys_window.hue(), sys_window.saturation(), sys_window.value() - 35);
        sys_window_tinted_hc2.setHsv(sys_window.hue(), sys_window.saturation(), sys_window.value() - 50);
    }

    // --- widget sizes ---------------------------------------------
    QFont fnt = QGuiApplication::font();
    auto  fm  = QFontMetrics(fnt);

    // todo: use precise values for ~9-11 point sizes
    unsigned font_small         = std::max(static_cast<unsigned>(fnt.pointSize() * 0.9), 8U);
    unsigned font_large         = static_cast<unsigned>(fnt.pointSize() * 1.8);
    unsigned text_height        = fm.height();
    unsigned text_padding       = static_cast<unsigned>(text_height * 0.10);
    unsigned text_padding_small = static_cast<unsigned>(text_height * 0.05);
    unsigned text_padding_large = static_cast<unsigned>(text_height * 0.25);

    // folderview top panel item sizes.
    unsigned top_panel_v_margin = 4;
    // Ensure at least 4px so it's not too thin.
    unsigned top_panel_text_padding = std::max(text_padding, 4U);
    // Scale with font, 38px base size.
    unsigned top_panel_height = std::max((text_height + top_panel_text_padding * 2 + top_panel_v_margin * 2), 38U);
    // Overlay headers.
    unsigned overlay_header_margin = 2;
    // 32px base size.
    unsigned overlay_header_size = std::max(text_height + text_padding * 2, 30U);
    // todo
    unsigned button_height = text_height + text_padding_large * 2;
    // Pseudo-dpi to scale some widget widths.
    unsigned text_height_base = 22;

    qreal pDpr = std::max((static_cast<qreal>(text_height) / text_height_base), 1.0);

    unsigned context_menu_width         = static_cast<unsigned>(212.0 * pDpr);
    unsigned context_menu_button_height = static_cast<unsigned>(32.0 * pDpr);
    unsigned rename_overlay_width       = static_cast<unsigned>(380.0 * pDpr);

    qDebug() << u"dpr=" << qApp->devicePixelRatio() << u"pDpr=" << pDpr;

    // --- write variables into stylesheet --------------------------
    styleSheet.replace(u"%font_small%"_s,                 QString::number(font_small) + u"pt");
    styleSheet.replace(u"%font_large%"_s,                 QString::number(font_large) + u"pt");
    styleSheet.replace(u"%button_height%"_s,              QString::number(button_height) + u"px");
    styleSheet.replace(u"%top_panel_height%"_s,           QString::number(top_panel_height) + u"px");
    styleSheet.replace(u"%overlay_header_size%"_s,        QString::number(overlay_header_size) + u"px");
    styleSheet.replace(u"%context_menu_width%"_s,         QString::number(context_menu_width) + u"px");
    styleSheet.replace(u"%context_menu_button_height%"_s, QString::number(context_menu_button_height) + u"px");
    styleSheet.replace(u"%rename_overlay_width%"_s,       QString::number(rename_overlay_width) + u"px");
    styleSheet.replace(u"%icontheme%"_s, u"light"_s);

    // Qt::Popup can't do transparency under windows, use square window
#ifdef Q_OS_WIN32
    styleSheet.replace(u"%contextmenu_border_radius%"_s, u"0px"_s);
#else
        styleSheet.replace(u"%contextmenu_border_radius%"_s, u"3px"_s);
#endif
    styleSheet.replace(u"%sys_window%"_s,            sys_window.name());
    styleSheet.replace(u"%sys_window_tinted%"_s,     sys_window_tinted.name());
    styleSheet.replace(u"%sys_window_tinted_lc%"_s,  sys_window_tinted_lc.name());
    styleSheet.replace(u"%sys_window_tinted_lc2%"_s, sys_window_tinted_lc2.name());
    styleSheet.replace(u"%sys_window_tinted_hc%"_s,  sys_window_tinted_hc.name());
    styleSheet.replace(u"%sys_window_tinted_hc2%"_s, sys_window_tinted_hc2.name());

    styleSheet.replace(u"%sys_text_secondary_rgba%"_s,
                       u"rgba(" + QString::number(sys_text.red()) + u',' +
                       QString::number(sys_text.green()) + u',' +
                       QString::number(sys_text.blue()) + u",50%)");

    styleSheet.replace(u"%button%"_s,                    colors.button.name());
    styleSheet.replace(u"%button_hover%"_s,              colors.button_hover.name());
    styleSheet.replace(u"%button_pressed%"_s,            colors.button_pressed.name());
    styleSheet.replace(u"%panel_button%"_s,              colors.panel_button.name());
    styleSheet.replace(u"%panel_button_hover%"_s,        colors.panel_button_hover.name());
    styleSheet.replace(u"%panel_button_pressed%"_s,      colors.panel_button_pressed.name());
    styleSheet.replace(u"%widget%"_s,                    colors.widget.name());
    styleSheet.replace(u"%widget_border%"_s,             colors.widget_border.name());
    styleSheet.replace(u"%folderview%"_s,                colors.folderview.name());
    styleSheet.replace(u"%folderview_topbar%"_s,         colors.folderview_topbar.name());
    styleSheet.replace(u"%folderview_hc%"_s,             colors.folderview_hc.name());
    styleSheet.replace(u"%folderview_hc2%"_s,            colors.folderview_hc2.name());
    styleSheet.replace(u"%accent%"_s,                    colors.accent.name());
    styleSheet.replace(u"%input_field_focus%"_s,         colors.input_field_focus.name());
    styleSheet.replace(u"%overlay%"_s,                   colors.overlay.name());
    styleSheet.replace(u"%icons%"_s,                     colors.icons.name());
    styleSheet.replace(u"%text_hc2%"_s,                  colors.text_hc2.name());
    styleSheet.replace(u"%text_hc%"_s,                   colors.text_hc.name());
    styleSheet.replace(u"%text%"_s,                      colors.text.name());
    styleSheet.replace(u"%overlay_text%"_s,              colors.overlay_text.name());
    styleSheet.replace(u"%text_lc%"_s,                   colors.text_lc.name());
    styleSheet.replace(u"%text_lc2%"_s,                  colors.text_lc2.name());
    styleSheet.replace(u"%scrollbar%"_s,                 colors.scrollbar.name());
    styleSheet.replace(u"%scrollbar_hover%"_s,           colors.scrollbar_hover.name());
    styleSheet.replace(u"%folderview_button_hover%"_s,   colors.folderview_button_hover.name());
    styleSheet.replace(u"%folderview_button_pressed%"_s, colors.folderview_button_pressed.name());

    styleSheet.replace(u"%text_secondary_rgba%"_s,
                       u"rgba(" + QString::number(colors.text.red()) + u',' +
                       QString::number(colors.text.green()) + u',' +
                       QString::number(colors.text.blue()) + u",62%)");
    styleSheet.replace(u"%accent_hover_rgba%"_s,
                       u"rgba(" + QString::number(colors.accent.red()) + u',' +
                       QString::number(colors.accent.green()) + u',' +
                       QString::number(colors.accent.blue()) + u",65%)");
    styleSheet.replace(u"%overlay_rgba%"_s,
                       u"rgba(" + QString::number(colors.overlay.red()) + u',' +
                       QString::number(colors.overlay.green()) + u',' +
                       QString::number(colors.overlay.blue()) + u",90%)");
    styleSheet.replace(u"%fv_backdrop_rgba%"_s,
                       u"rgba(" + QString::number(colors.folderview_hc2.red()) + u',' +
                       QString::number(colors.folderview_hc2.green()) + u',' +
                       QString::number(colors.folderview_hc2.blue()) + u",80%)");

    // do not show separator line if topbar color matches folderview
    styleSheet.replace(u"%topbar_border_rgba%"_s,
                       colors.folderview == colors.folderview_topbar ? colors.folderview.name()
                           : u"rgba(0,0,0,14%)"_s);

    // --- apply -------------------------------------------------
    qApp->setStyleSheet(styleSheet);
}

//------------------------------------------------------------------------------

void Settings::loadTheme()
{
    if (useSystemColorScheme()) {
        setColorScheme(ThemeStore::colorScheme(ColorSchemes::SYSTEM));
    } else {
        BaseColorScheme base;
        themeConf.beginGroup(u"Colors"_sv);
        base.background            = QColor(themeConf.value(u"background"_sv, u"#1a1a1a"_s).toString());
        base.background_fullscreen = QColor(themeConf.value(u"background_fullscreen"_sv, u"#1a1a1a"_s).toString());
        base.text                  = QColor(themeConf.value(u"text"_sv, u"#b6b6b6"_s).toString());
        base.icons                 = QColor(themeConf.value(u"icons"_sv, u"#a4a4a4"_s).toString());
        base.widget                = QColor(themeConf.value(u"widget"_sv, u"#252525"_s).toString());
        base.widget_border         = QColor(themeConf.value(u"widget_border"_sv, u"#2c2c2c"_s).toString());
        base.accent                = QColor(themeConf.value(u"accent"_sv, u"#8c9b81"_s).toString());
        base.folderview            = QColor(themeConf.value(u"folderview"_sv, u"#242424"_s).toString());
        base.folderview_topbar     = QColor(themeConf.value(u"folderview_topbar"_sv, u"#383838"_s).toString());
        base.scrollbar             = QColor(themeConf.value(u"scrollbar"_sv, u"#5a5a5a"_s).toString());
        base.overlay_text          = QColor(themeConf.value(u"overlay_text"_sv, u"#d2d2d2"_s).toString());
        base.overlay               = QColor(themeConf.value(u"overlay"_sv, u"#1a1a1a"_s).toString());
        base.tid                   = themeConf.value(u"tid"_sv, u"-1"_s).toInt();
        themeConf.endGroup();
        setColorScheme(ColorScheme(base));
    }
}
void Settings::saveTheme()
{
    if (useSystemColorScheme())
        return;
    themeConf.beginGroup(u"Colors");
    themeConf.setValue(u"background"_sv,            mColorScheme.background.name());
    themeConf.setValue(u"background_fullscreen"_sv, mColorScheme.background_fullscreen.name());
    themeConf.setValue(u"text"_sv,                  mColorScheme.text.name());
    themeConf.setValue(u"icons"_sv,                 mColorScheme.icons.name());
    themeConf.setValue(u"widget"_sv,                mColorScheme.widget.name());
    themeConf.setValue(u"widget_border"_sv,         mColorScheme.widget_border.name());
    themeConf.setValue(u"accent"_sv,                mColorScheme.accent.name());
    themeConf.setValue(u"folderview"_sv,            mColorScheme.folderview.name());
    themeConf.setValue(u"folderview_topbar"_sv,     mColorScheme.folderview_topbar.name());
    themeConf.setValue(u"scrollbar"_sv,             mColorScheme.scrollbar.name());
    themeConf.setValue(u"overlay_text"_sv,          mColorScheme.overlay_text.name());
    themeConf.setValue(u"overlay"_sv,               mColorScheme.overlay.name());
    themeConf.setValue(u"tid"_sv,                   mColorScheme.tid);
    themeConf.endGroup();
}

//------------------------------------------------------------------------------

ColorScheme const &Settings::colorScheme() const
{
    return mColorScheme;
}

void Settings::setColorScheme(ColorScheme const &scheme)
{
    mColorScheme = scheme;
    loadStylesheet();
}

//------------------------------------------------------------------------------

void Settings::setColorTid(int tid)
{
    mColorScheme.tid = tid;
}

void Settings::setColorTid(ColorSchemes tid)
{
    setColorTid(static_cast<int>(tid));
}

//------------------------------------------------------------------------------
void Settings::fillVideoFormats() {
    mVideoFormatsMap.insert("video/webm"_ba,       "webm"_ba);
    mVideoFormatsMap.insert("video/mp4"_ba,        "mp4"_ba);
    mVideoFormatsMap.insert("video/mp4"_ba,        "m4v"_ba);
    mVideoFormatsMap.insert("video/mpeg"_ba,       "mpg"_ba);
    mVideoFormatsMap.insert("video/mpeg"_ba,       "mpeg"_ba);
    mVideoFormatsMap.insert("video/x-matroska"_ba, "mkv"_ba);
    mVideoFormatsMap.insert("video/x-ms-wmv"_ba,   "wmv"_ba);
    mVideoFormatsMap.insert("video/x-msvideo"_ba,  "avi"_ba);
    mVideoFormatsMap.insert("video/quicktime"_ba,  "mov"_ba);
    mVideoFormatsMap.insert("video/x-flv"_ba,      "flv"_ba);
}

//------------------------------------------------------------------------------

QString Settings::mpvBinary() const
{
    QString mpvPath = settingsConf.value(u"mpvBinary"_sv, u""_s).toString();
    if (!QFile::exists(mpvPath)) {
#ifdef Q_OS_WIN32
        mpvPath = QCoreApplication::applicationDirPath() + u"/mpv.exe";
#elif defined __linux__
        mpvPath = u"/usr/bin/mpv"_s;
#endif
        if (!QFile::exists(mpvPath))
            mpvPath = u""_s;
    }
    return mpvPath;
}

void Settings::setMpvBinary(QString const &path)
{
    if (QFile::exists(path))
        settingsConf.setValue(u"mpvBinary"_sv, path);
}

//------------------------------------------------------------------------------

QList<QByteArray> Settings::supportedFormats() const
{
    auto formats = QImageReader::supportedImageFormats();
    formats << "jfif"_ba;
    if (videoPlayback())
        formats << mVideoFormatsMap.values();
    formats.removeAll("pdf");
    return formats;
}

// (for open/save dialogs, as a single string)
// example:  u"Images (*.jpg, *.png)"_s
QString Settings::supportedFormatsFilter() const
{
    QString filters;
    auto    formats = supportedFormats();
    filters.append(u"Supported files ("_sv);
    for (auto &fmt : formats)
        filters.append(u"*." + QString::fromUtf8(fmt) + u' ');
    filters.append(u')');
    return filters;
}

QString Settings::supportedFormatsRegex() const
{
    QString           filter;
    QList<QByteArray> formats = supportedFormats();
    filter.append(u".*\\.("_sv);
    for (auto &fmt : formats)
        filter.append(QString::fromUtf8(fmt) + u'|');
    filter.chop(1);
    filter.append(u")$"_sv);
    return filter;
}

// returns list of mime types
QStringList Settings::supportedMimeTypes() const
{
    QStringList       filters;
    QList<QByteArray> mimeTypes = QImageReader::supportedMimeTypes();
    if (videoPlayback())
        mimeTypes << mVideoFormatsMap.keys();
    for (auto const &type : mimeTypes)
        filters << QString::fromLatin1(type);
    return filters;
}

//------------------------------------------------------------------------------

bool Settings::videoPlayback() const
{
#ifdef USE_MPV
    return settingsConf.value(u"videoPlayback"_sv, true).toBool();
#else
    return false;
#endif
}

void Settings::setVideoPlayback(bool mode)
{
    settingsConf.setValue(u"videoPlayback"_sv, mode);
}

//------------------------------------------------------------------------------

bool Settings::useSystemColorScheme() const
{
    return settingsConf.value(u"useSystemColorScheme", false).toBool();
}

void Settings::setUseSystemColorScheme(bool mode)
{
    settingsConf.setValue(u"useSystemColorScheme"_sv, mode);
}

//------------------------------------------------------------------------------

QVersionNumber Settings::lastVersion() const
{
    int vmajor = settingsConf.value(u"lastVerMajor"_sv, 0).toInt();
    int vminor = settingsConf.value(u"lastVerMinor"_sv, 0).toInt();
    int vmicro = settingsConf.value(u"lastVerMicro"_sv, 0).toInt();
    return QVersionNumber(vmajor, vminor, vmicro);
}

void Settings::setLastVersion(QVersionNumber const &ver)
{
    settingsConf.setValue(u"lastVerMajor"_sv, ver.majorVersion());
    settingsConf.setValue(u"lastVerMinor"_sv, ver.minorVersion());
    settingsConf.setValue(u"lastVerMicro"_sv, ver.microVersion());
}

//------------------------------------------------------------------------------

void Settings::setShowChangelogs(bool mode)
{
    settingsConf.setValue(u"showChangelogs"_sv, mode);
}

bool Settings::showChangelogs() const
{
    return settingsConf.value(u"showChangelogs"_sv, true).toBool();
}

//------------------------------------------------------------------------------

qreal Settings::backgroundOpacity() const
{
    bool  ok    = false;
    qreal value = settingsConf.value(u"backgroundOpacity"_sv, 1.0).toReal(&ok);
    if (!ok)
        return 0.0;
    if (value > 1.0)
        return 1.0;
    if (value < 0.0)
        return 0.0;
    return value;
}

void Settings::setBackgroundOpacity(qreal value)
{
    if (value > 1.0)
        value = 1.0;
    else if (value < 0.0)
        value = 0.0;
    settingsConf.setValue(u"backgroundOpacity"_sv, value);
}

//------------------------------------------------------------------------------

bool Settings::blurBackground() const
{
#ifdef USE_KDE_BLUR
    return settingsConf.value(u"blurBackground"_sv, true).toBool();
#else
    return false;
#endif
}

void Settings::setBlurBackground(bool mode)
{
    settingsConf.setValue(u"blurBackground"_sv, mode);
}
//------------------------------------------------------------------------------
void Settings::setSortingMode(SortingMode mode)
{
    if (mode > SortingMode::TIME_DESC)
        mode = SortingMode::NAME;
    settingsConf.setValue(u"sortingMode"_sv, static_cast<int>(mode));
}

SortingMode Settings::sortingMode() const
{
    int mode = settingsConf.value(u"sortingMode"_sv, 0).toInt();
    if (mode < 0 || mode >= 6)
        mode = 0;
    return static_cast<SortingMode>(mode);
}
//------------------------------------------------------------------------------
bool Settings::playVideoSounds() const
{
    return settingsConf.value(u"playVideoSounds"_sv, false).toBool();
}

void Settings::setPlayVideoSounds(bool mode)
{
    settingsConf.setValue(u"playVideoSounds"_sv, mode);
}
//------------------------------------------------------------------------------
void Settings::setVolume(int vol)
{
    stateConf.setValue(u"volume"_sv, vol);
}

int Settings::volume() const
{
    return stateConf.value(u"volume"_sv, 100).toInt();
}
//------------------------------------------------------------------------------
FolderViewMode Settings::folderViewMode() const
{
    int mode = settingsConf.value(u"folderViewMode"_sv, 2).toInt();
    if (mode < 0 || mode >= 3)
        mode = 2;
    return static_cast<FolderViewMode>(mode);
}

void Settings::setFolderViewMode(FolderViewMode mode)
{
    settingsConf.setValue(u"folderViewMode"_sv, static_cast<int>(mode));
}
//------------------------------------------------------------------------------
ThumbPanelStyle Settings::thumbPanelStyle() const
{
    int mode = settingsConf.value(u"thumbPanelStyle"_sv, 1).toInt();
    if (mode < 0 || mode > 1)
        mode = 1;
    return static_cast<ThumbPanelStyle>(mode);
}

void Settings::setThumbPanelStyle(ThumbPanelStyle mode)
{
    settingsConf.setValue(u"thumbPanelStyle"_sv, static_cast<int>(mode));
}
//------------------------------------------------------------------------------
QMultiMap<QByteArray, QByteArray> const &Settings::videoFormats() const
{
    return mVideoFormatsMap;
}
//------------------------------------------------------------------------------
int Settings::panelPreviewsSize() const
{
    bool ok   = true;
    int  size = settingsConf.value(u"panelPreviewsSize"_sv, 140).toInt(&ok);
    if (!ok)
        size = 140;
    size = qBound(100, size, 250);
    return size;
}

void Settings::setPanelPreviewsSize(int size)
{
    settingsConf.setValue(u"panelPreviewsSize"_sv, size);
}
//------------------------------------------------------------------------------
bool Settings::usePreloader() const
{
    return settingsConf.value(u"usePreloader"_sv, true).toBool();
}

void Settings::setUsePreloader(bool mode)
{
    settingsConf.setValue(u"usePreloader"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::keepFitMode() const
{
    return settingsConf.value(u"keepFitMode"_sv, false).toBool();
}

void Settings::setKeepFitMode(bool mode)
{
    settingsConf.setValue(u"keepFitMode"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::fullscreenMode() const
{
    return settingsConf.value(u"openInFullscreen"_sv, false).toBool();
}

void Settings::setFullscreenMode(bool mode)
{
    settingsConf.setValue(u"openInFullscreen"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::maximizedWindow() const
{
    return stateConf.value(u"maximizedWindow"_sv, false).toBool();
}

void Settings::setMaximizedWindow(bool mode)
{
    stateConf.setValue(u"maximizedWindow"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::panelEnabled() const
{
    return settingsConf.value(u"panelEnabled"_sv, true).toBool();
}

void Settings::setPanelEnabled(bool mode)
{
    settingsConf.setValue(u"panelEnabled"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::panelFullscreenOnly() const
{
    return settingsConf.value(u"panelFullscreenOnly"_sv, true).toBool();
}

void Settings::setPanelFullscreenOnly(bool mode)
{
    settingsConf.setValue(u"panelFullscreenOnly"_sv, mode);
}
//------------------------------------------------------------------------------
int Settings::lastDisplay() const
{
    return stateConf.value(u"lastDisplay"_sv, 0).toInt();
}

void Settings::setLastDisplay(int display)
{
    stateConf.setValue(u"lastDisplay"_sv, display);
}
//------------------------------------------------------------------------------
PanelPosition Settings::panelPosition() const
{
    QString posString = settingsConf.value(u"panelPosition"_sv, u"top"_s).toString();
    if (posString == u"top"_sv)
        return PanelPosition::TOP;
    if (posString == u"bottom"_sv)
        return PanelPosition::BOTTOM;
    if (posString == u"left"_sv)
        return PanelPosition::LEFT;
    return PanelPosition::RIGHT;
}

void Settings::setPanelPosition(PanelPosition pos)
{
    QString posString;
    switch (pos) {
    case PanelPosition::TOP:    posString = u"top"_s;    break;
    case PanelPosition::BOTTOM: posString = u"bottom"_s; break;
    case PanelPosition::LEFT:   posString = u"left"_s;   break;
    case PanelPosition::RIGHT:  posString = u"right"_s;  break;
    }
    settingsConf.setValue(u"panelPosition", std::move(posString));
}
//------------------------------------------------------------------------------
bool Settings::panelPinned() const
{
    return settingsConf.value(u"panelPinned"_sv, false).toBool();
}

void Settings::setPanelPinned(bool mode)
{
    settingsConf.setValue(u"panelPinned"_sv, mode);
}
//------------------------------------------------------------------------------
/*
 * 0: fit window
 * 1: fit width
 * 2: orginal size
 */
ImageFitMode Settings::imageFitMode() const
{
    int mode = settingsConf.value(u"defaultFitMode"_sv, 0).toInt();
    if (mode < 0 || mode > 2) {
        qDebug() << u"Settings: Invalid fit mode ( " << QString::number(mode) << u" ). Resetting to default.";
        mode = 0;
    }
    return static_cast<ImageFitMode>(mode);
}

void Settings::setImageFitMode(ImageFitMode mode)
{
    int modeInt = static_cast<int>(mode);
    if (modeInt < 0 || modeInt > 2) {
        qDebug() << u"Settings: Invalid fit mode ( " << QString::number(modeInt) << u" ). Resetting to default.";
        modeInt = 0;
    }
    settingsConf.setValue(u"defaultFitMode", modeInt);
}
//------------------------------------------------------------------------------
QRect Settings::windowGeometry() const
{
    QRect savedRect = stateConf.value(u"windowGeometry"_sv).toRect();
    if (savedRect.size().isEmpty())
        savedRect.setRect(100, 100, 900, 600);
    return savedRect;
}

void Settings::setWindowGeometry(QRect geometry)
{
    stateConf.setValue(u"windowGeometry"_sv, geometry);
}
//------------------------------------------------------------------------------
bool Settings::loopSlideshow() const
{
    return settingsConf.value(u"loopSlideshow"_sv, false).toBool();
}

void Settings::setLoopSlideshow(bool mode)
{
    settingsConf.setValue(u"loopSlideshow"_sv, mode);
}
//------------------------------------------------------------------------------
void Settings::sendChangeNotification()
{
    emit settingsChanged();
}
//------------------------------------------------------------------------------
void Settings::readShortcuts(QMap<QString, QString> &shortcuts)
{
    settingsConf.beginGroup(u"Controls"_s);
    QStringList in = settingsConf.value(u"shortcuts"_sv).toStringList();

    for (auto &item : in) {
        QStringList pair = item.split(u'=');
        if (!pair[0].isEmpty() && !pair[1].isEmpty()) {
            if (pair[1] == u"eq"_sv)
                pair[1] = u'=';
            shortcuts.insert(pair[1], pair[0]);
        }
    }

    settingsConf.endGroup();
}

void Settings::saveShortcuts(QMap<QString, QString> const &shortcuts)
{
    settingsConf.beginGroup(u"Controls"_sv);
    QMapIterator i(shortcuts);
    QStringList  out;
    while (i.hasNext()) {
        i.next();
        if (i.key() == u"="_sv)
            out << i.value() + u'=' + u"eq";
        else
            out << i.value() + u'=' + i.key();
    }
    settingsConf.setValue(u"shortcuts"_sv, out);
    settingsConf.endGroup();
}
//------------------------------------------------------------------------------
void Settings::readScripts(QMap<QString, Script> &scripts)
{
    scripts.clear();
    settingsConf.beginGroup(u"Scripts"_sv);
    int size = settingsConf.beginReadArray(u"script"_sv);
    for (int i = 0; i < size; ++i) {
        settingsConf.setArrayIndex(i);
        QString  name  = settingsConf.value(u"name"_sv).toString();
        QVariant value = settingsConf.value(u"value"_sv);
        Script   scr   = value.value<Script>();
        scripts.insert(name, scr);
    }
    settingsConf.endArray();
    settingsConf.endGroup();
}

void Settings::saveScripts(QMap<QString, Script> const &scripts)
{
    settingsConf.beginGroup(u"Scripts"_sv);
    settingsConf.beginWriteArray(u"script"_sv);
    QMapIterator i(scripts);
    unsigned     counter = 0;
    while (i.hasNext()) {
        i.next();
        settingsConf.setArrayIndex(counter);
        settingsConf.setValue(u"name"_sv, i.key());
        settingsConf.setValue(u"value"_sv, QVariant::fromValue(i.value()));
        counter++;
    }
    settingsConf.endArray();
    settingsConf.endGroup();
}
//------------------------------------------------------------------------------
bool Settings::squareThumbnails() const
{
    return settingsConf.value(u"squareThumbnails"_sv, false).toBool();
}

void Settings::setSquareThumbnails(bool mode)
{
    settingsConf.setValue(u"squareThumbnails"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::transparencyGrid() const
{
    return settingsConf.value(u"drawTransparencyGrid"_sv, false).toBool();
}

void Settings::setTransparencyGrid(bool mode)
{
    settingsConf.setValue(u"drawTransparencyGrid"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::enableSmoothScroll() const
{
    return settingsConf.value(u"enableSmoothScroll"_sv, true).toBool();
}

void Settings::setEnableSmoothScroll(bool mode)
{
    settingsConf.setValue(u"enableSmoothScroll"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::useThumbnailCache() const
{
    return settingsConf.value(u"thumbnailCache"_sv, true).toBool();
}

void Settings::setUseThumbnailCache(bool mode)
{
    settingsConf.setValue(u"thumbnailCache"_sv, mode);
}
//------------------------------------------------------------------------------
QStringList Settings::savedPaths() const
{
    return stateConf.value(u"savedPaths"_sv, QDir::homePath()).toStringList();
}

void Settings::setSavedPaths(QStringList const &paths)
{
    stateConf.setValue(u"savedPaths"_sv, paths);
}
//------------------------------------------------------------------------------
QStringList Settings::bookmarks() const
{
    return stateConf.value(u"bookmarks").toStringList();
}

void Settings::setBookmarks(QStringList const &paths)
{
    stateConf.setValue(u"bookmarks"_sv, paths);
}
//------------------------------------------------------------------------------
bool Settings::placesPanel() const
{
    return stateConf.value(u"placesPanel"_sv, true).toBool();
}

void Settings::setPlacesPanel(bool mode)
{
    stateConf.setValue(u"placesPanel"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::placesPanelBookmarksExpanded() const
{
    return stateConf.value(u"placesPanelBookmarksExpanded"_sv, true).toBool();
}

void Settings::setPlacesPanelBookmarksExpanded(bool mode)
{
    stateConf.setValue(u"placesPanelBookmarksExpanded"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::placesPanelTreeExpanded() const
{
    return stateConf.value(u"placesPanelTreeExpanded"_sv, true).toBool();
}

void Settings::setPlacesPanelTreeExpanded(bool mode)
{
    stateConf.setValue(u"placesPanelTreeExpanded"_sv, mode);
}
//------------------------------------------------------------------------------
int Settings::placesPanelWidth() const
{
    return stateConf.value(u"placesPanelWidth"_sv, 260).toInt();
}

void Settings::setPlacesPanelWidth(int width)
{
    stateConf.setValue(u"placesPanelWidth"_sv, width);
}
//------------------------------------------------------------------------------
void Settings::setSlideshowInterval(int ms)
{
    settingsConf.setValue(u"slideshowInterval"_sv, ms);
}

int Settings::slideshowInterval() const
{
    int interval = settingsConf.value(u"slideshowInterval"_sv, 3000).toInt();
    if (interval <= 0)
        interval = 3000;
    return interval;
}
//------------------------------------------------------------------------------
int Settings::thumbnailerThreadCount() const
{
    int count = settingsConf.value(u"thumbnailerThreads"_sv, 4).toInt();
    if (count < 1)
        count = 4;
    return count;
}

void Settings::setThumbnailerThreadCount(int count)
{
    settingsConf.setValue(u"thumbnailerThreads"_sv, count);
}
//------------------------------------------------------------------------------
bool Settings::smoothUpscaling() const
{
    return settingsConf.value(u"smoothUpscaling"_sv, true).toBool();
}

void Settings::setSmoothUpscaling(bool mode)
{
    settingsConf.setValue(u"smoothUpscaling"_sv, mode);
}
//------------------------------------------------------------------------------
int Settings::folderViewIconSize() const
{
    return settingsConf.value(u"folderViewIconSize"_sv, 120).toInt();
}

void Settings::setFolderViewIconSize(int value)
{
    settingsConf.setValue(u"folderViewIconSize"_sv, value);
}
//------------------------------------------------------------------------------
bool Settings::expandImage() const
{
    return settingsConf.value(u"expandImage"_sv, false).toBool();
}

void Settings::setExpandImage(bool mode)
{
    settingsConf.setValue(u"expandImage"_sv, mode);
}
//------------------------------------------------------------------------------
int Settings::expandLimit() const
{
    return settingsConf.value(u"expandLimit"_sv, 2).toInt();
}

void Settings::setExpandLimit(int value)
{
    settingsConf.setValue(u"expandLimit"_sv, value);
}
//------------------------------------------------------------------------------
int Settings::JPEGSaveQuality() const
{
    int quality = std::clamp(settingsConf.value(u"JPEGSaveQuality"_sv, 95).toInt(), 0, 100);
    return quality;
}

void Settings::setJPEGSaveQuality(int value)
{
    settingsConf.setValue(u"JPEGSaveQuality"_sv, value);
}
//------------------------------------------------------------------------------
ScalingFilter Settings::scalingFilter() const
{
    int defaultFilter = 1;
#ifdef USE_OPENCV
    // default to a nicer CV_CUBIC
    defaultFilter = 3;
#endif
    int mode = settingsConf.value(u"scalingFilter"_sv, defaultFilter).toInt();
#ifndef USE_OPENCV
    if (mode > 2)
        mode = 1;
#endif
    if (mode < 0 || mode > 4)
        mode = 1;
    return static_cast<ScalingFilter>(mode);
}

void Settings::setScalingFilter(ScalingFilter mode)
{
    settingsConf.setValue(u"scalingFilter"_sv, static_cast<int>(mode));
}
//------------------------------------------------------------------------------
bool Settings::smoothAnimatedImages() const
{
    return settingsConf.value(u"smoothAnimatedImages"_sv, true).toBool();
}

void Settings::setSmoothAnimatedImages(bool mode)
{
    settingsConf.setValue(u"smoothAnimatedImages"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::infoBarFullscreen() const
{
    return settingsConf.value(u"infoBarFullscreen"_sv, true).toBool();
}

void Settings::setInfoBarFullscreen(bool mode)
{
    settingsConf.setValue(u"infoBarFullscreen"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::infoBarWindowed() const
{
    return settingsConf.value(u"infoBarWindowed"_sv, false).toBool();
}

void Settings::setInfoBarWindowed(bool mode)
{
    settingsConf.setValue(u"infoBarWindowed"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::windowTitleExtendedInfo() const
{
    return settingsConf.value(u"windowTitleExtendedInfo"_sv, true).toBool();
}

void Settings::setWindowTitleExtendedInfo(bool mode)
{
    settingsConf.setValue(u"windowTitleExtendedInfo"_sv, mode);
}

//------------------------------------------------------------------------------
bool Settings::cursorAutohide() const
{
    return settingsConf.value(u"cursorAutohiding"_sv, true).toBool();
}

void Settings::setCursorAutohide(bool mode)
{
    settingsConf.setValue(u"cursorAutohiding"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::firstRun() const
{
    return settingsConf.value(u"firstRun"_sv, true).toBool();
}

void Settings::setFirstRun(bool mode)
{
    settingsConf.setValue(u"firstRun"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::showSaveOverlay() const
{
    return settingsConf.value(u"showSaveOverlay"_sv, true).toBool();
}

void Settings::setShowSaveOverlay(bool mode)
{
    settingsConf.setValue(u"showSaveOverlay"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::confirmDelete() const
{
    return settingsConf.value(u"confirmDelete"_sv, true).toBool();
}

void Settings::setConfirmDelete(bool mode)
{
    settingsConf.setValue(u"confirmDelete"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::confirmTrash() const
{
    return settingsConf.value(u"confirmTrash"_sv, true).toBool();
}

void Settings::setConfirmTrash(bool mode)
{
    settingsConf.setValue(u"confirmTrash"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::unloadThumbs() const
{
    return settingsConf.value(u"unloadThumbs"_sv, true).toBool();
}

void Settings::setUnloadThumbs(bool mode)
{
    settingsConf.setValue(u"unloadThumbs"_sv, mode);
}
//------------------------------------------------------------------------------
float Settings::zoomStep() const
{
    bool  ok    = false;
    float value = settingsConf.value(u"zoomStep"_sv, 0.2f).toFloat(&ok);
    if (!ok)
        return 0.2f;
    value = qBound(0.01f, value, 0.5f);
    return value;
}

void Settings::setZoomStep(float value)
{
    value = qBound(0.01f, value, 0.5f);
    settingsConf.setValue(u"zoomStep"_sv, value);
}
//------------------------------------------------------------------------------
void Settings::setZoomIndicatorMode(ZoomIndicatorMode mode)
{
    settingsConf.setValue(u"zoomIndicatorMode"_sv, static_cast<int>(mode));
}

ZoomIndicatorMode Settings::zoomIndicatorMode() const
{
    int mode = settingsConf.value(u"zoomIndicatorMode"_sv, 0).toInt();
    if (mode < 0 || mode > 2)
        mode = 0;
    return static_cast<ZoomIndicatorMode>(mode);
}
//------------------------------------------------------------------------------
void Settings::setFocusPointIn1to1Mode(ImageFocusPoint mode)
{
    settingsConf.setValue(u"focusPointIn1to1Mode"_sv, static_cast<int>(mode));
}

ImageFocusPoint Settings::focusPointIn1to1Mode() const
{
    int mode = settingsConf.value(u"focusPointIn1to1Mode"_sv, 1).toInt();
    if (mode < 0 || mode > 2)
        mode = 1;
    return static_cast<ImageFocusPoint>(mode);
}

void Settings::setDefaultCropAction(DefaultCropAction mode)
{
    settingsConf.setValue(u"defaultCropAction"_sv, static_cast<int>(mode));
}

DefaultCropAction Settings::defaultCropAction() const
{
    int mode = settingsConf.value(u"defaultCropAction"_sv, 0).toInt();
    if (mode < 0 || mode > 1)
        mode = 0;
    return static_cast<DefaultCropAction>(mode);
}

ImageScrolling Settings::imageScrolling() const
{
    int mode = settingsConf.value(u"imageScrolling"_sv, 1).toInt();
    if (mode < 0 || mode > 2)
        mode = 0;
    return static_cast<ImageScrolling>(mode);
}

void Settings::setImageScrolling(ImageScrolling mode)
{
    settingsConf.setValue(u"imageScrolling"_sv, static_cast<int>(mode));
}
//------------------------------------------------------------------------------
ViewMode Settings::defaultViewMode() const
{
    int mode = settingsConf.value(u"defaultViewMode"_sv, 0).toInt();
    if (mode < 0 || mode > 1)
        mode = 0;
    return static_cast<ViewMode>(mode);
}

void Settings::setDefaultViewMode(ViewMode mode)
{
    settingsConf.setValue(u"defaultViewMode"_sv, static_cast<int>(mode));
}
//------------------------------------------------------------------------------
FolderEndAction Settings::folderEndAction() const
{
    int mode = settingsConf.value(u"folderEndAction"_sv, 0).toInt();
    if (mode < 0 || mode > 2)
        mode = 0;
    return static_cast<FolderEndAction>(mode);
}

void Settings::setFolderEndAction(FolderEndAction mode)
{
    settingsConf.setValue(u"folderEndAction"_sv, static_cast<int>(mode));
}
//------------------------------------------------------------------------------
bool Settings::printLandscape() const
{
    return stateConf.value(u"printLandscape"_sv, false).toBool();
}

void Settings::setPrintLandscape(bool mode)
{
    stateConf.setValue(u"printLandscape"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::printPdfDefault() const
{
    return stateConf.value(u"printPdfDefault"_sv, false).toBool();
}

void Settings::setPrintPdfDefault(bool mode)
{
    stateConf.setValue(u"printPdfDefault"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::printColor() const
{
    return stateConf.value(u"printColor"_sv, false).toBool();
}

void Settings::setPrintColor(bool mode)
{
    stateConf.setValue(u"printColor"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::printFitToPage() const
{
    return stateConf.value(u"printFitToPage"_sv, true).toBool();
}

void Settings::setPrintFitToPage(bool mode)
{
    stateConf.setValue(u"printFitToPage"_sv, mode);
}
//------------------------------------------------------------------------------
QString Settings::lastPrinter() const
{
    return stateConf.value(u"lastPrinter"_sv, u""_s).toString();
}

void Settings::setLastPrinter(QString const &name)
{
    stateConf.setValue(u"lastPrinter"_sv, name);
}
//------------------------------------------------------------------------------
bool Settings::jxlAnimation() const
{
    return settingsConf.value(u"jxlAnimation"_sv, false).toBool();
}

void Settings::setJxlAnimation(bool mode)
{
    settingsConf.setValue(u"jxlAnimation"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::autoResizeWindow() const
{
    return settingsConf.value(u"autoResizeWindow"_sv, false).toBool();
}

void Settings::setAutoResizeWindow(bool mode)
{
    settingsConf.setValue(u"autoResizeWindow"_sv, mode);
}
//------------------------------------------------------------------------------
int Settings::autoResizeLimit() const
{
    int limit = settingsConf.value(u"autoResizeLimit"_sv, 90).toInt();
    if (limit < 30 || limit > 100)
        limit = 90;
    return limit;
}

void Settings::setAutoResizeLimit(int percent)
{
    settingsConf.setValue(u"autoResizeLimit"_sv, percent);
}
//------------------------------------------------------------------------------
int Settings::memoryAllocationLimit() const
{
    int limit = settingsConf.value(u"memoryAllocationLimit"_sv, 1024).toInt();
    if (limit < 512)
        limit = 512;
    else if (limit > 8192)
        limit = 8192;
    return limit;
}

void Settings::setMemoryAllocationLimit(int limitMB)
{
    settingsConf.setValue(u"memoryAllocationLimit"_sv, limitMB);
}
//------------------------------------------------------------------------------
bool Settings::panelCenterSelection() const
{
    return settingsConf.value(u"panelCenterSelection"_sv, false).toBool();
}

void Settings::setPanelCenterSelection(bool mode)
{
    settingsConf.setValue(u"panelCenterSelection"_sv, mode);
}
//------------------------------------------------------------------------------
QString Settings::language() const
{
    return settingsConf.value(u"language"_sv, u"en_US"_s).toString();
}

void Settings::setLanguage(QString const &lang)
{
    settingsConf.setValue(u"language"_sv, lang);
}
//------------------------------------------------------------------------------
bool Settings::useFixedZoomLevels() const
{
    return settingsConf.value(u"useFixedZoomLevels"_sv, false).toBool();
}

void Settings::setUseFixedZoomLevels(bool mode)
{
    settingsConf.setValue(u"useFixedZoomLevels"_sv, mode);
}
//------------------------------------------------------------------------------
QString Settings::defaultZoomLevels()
{
    return u"0.05,0.1,0.125,0.166,0.25,0.333,0.5,0.66,1,1.5,2,3,4,5,6,7,8"_s;
}
QString Settings::zoomLevels() const
{
    return settingsConf.value(u"fixedZoomLevels"_sv, defaultZoomLevels()).toString();
}

void Settings::setZoomLevels(QString const &levels)
{
    settingsConf.setValue(u"fixedZoomLevels"_sv, levels);
}
//------------------------------------------------------------------------------
bool Settings::unlockMinZoom() const
{
    return settingsConf.value(u"unlockMinZoom"_sv, true).toBool();
}

void Settings::setUnlockMinZoom(bool mode)
{
    settingsConf.setValue(u"unlockMinZoom"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::sortFolders() const
{
    return settingsConf.value(u"sortFolders"_sv, true).toBool();
}

void Settings::setSortFolders(bool mode)
{
    settingsConf.setValue(u"sortFolders"_sv, mode);
}
//------------------------------------------------------------------------------
bool Settings::trackpadDetection() const
{
    return settingsConf.value(u"trackpadDetection"_sv, true).toBool();
}

void Settings::setTrackpadDetection(bool mode)
{
    settingsConf.setValue(u"trackpadDetection"_sv, mode);
}
