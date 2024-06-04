#include "settings.h"

Settings *settings = nullptr;

Settings::Settings(QObject *parent)
    : QObject(parent),
#ifdef Q_OS_LINUX
      stateConf(QCoreApplication::organizationName(), QS("savedState")),
      themeConf(QCoreApplication::organizationName(), QS("theme"))
#else
      mConfDir(QApplication::applicationDirPath() + QSV("/conf")),
      settingsConf(mConfDir.absolutePath() + u'/' + qApp->applicationName() + QSV(".ini"), QSettings::IniFormat),
      stateConf   (mConfDir.absolutePath() + QSV("/savedState.ini"), QSettings::IniFormat),
      themeConf   (mConfDir.absolutePath() + QSV("/theme.ini"), QSettings::IniFormat)
#endif
{
    // config files
#ifdef Q_OS_LINUX
#else
    mConfDir.mkpath(QApplication::applicationDirPath() + QSV("/conf"));
#endif

    fillVideoFormats();
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
void Settings::setupCache()
{
#ifdef Q_OS_LINUX
    QString genericCacheLocation = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    if (genericCacheLocation.isEmpty())
        genericCacheLocation = QDir::homePath() + QS("/.cache");
    genericCacheLocation.append(QS("/") + QApplication::applicationName());
    mTmpDir = QDir(settingsConf.value(QSV("cacheDir"), genericCacheLocation).toString());
    mTmpDir.mkpath(mTmpDir.absolutePath());

    QFileInfo dirTest(mTmpDir.absolutePath());
    if (!dirTest.isDir() || !dirTest.isWritable() || !dirTest.exists()) {
        // fallback
        qDebug() << u"Error: cache dir is not writable" << mTmpDir.absolutePath();
        qDebug() << u"Trying to use" << genericCacheLocation << u"instead";
        mTmpDir.setPath(genericCacheLocation);
        mTmpDir.mkpath(mTmpDir.absolutePath());
    }
    mThumbCacheDir = QDir(mTmpDir.absolutePath() + QS("/thumbnails"));
    mThumbCacheDir.mkpath(mThumbCacheDir.absolutePath());
#else
    mTmpDir = QDir(QApplication::applicationDirPath() + QSV("/cache"));
    mTmpDir.mkpath(mTmpDir.absolutePath());
    mThumbCacheDir = QDir(QApplication::applicationDirPath() + QSV("/thumbnails"));
    mThumbCacheDir.mkpath(mThumbCacheDir.absolutePath());
#endif
}
//------------------------------------------------------------------------------
void Settings::sync()
{
    settingsConf.sync();
    stateConf.sync();
}
//------------------------------------------------------------------------------
QString Settings::thumbnailCacheDir() const
{
    return mThumbCacheDir.path() + u'/';
}
//------------------------------------------------------------------------------
QString Settings::tmpDir() const
{
    return mTmpDir.path() + u'/';
}
//------------------------------------------------------------------------------
// This here is temporarily, will be moved to some sort of theme manager class.
void Settings::loadStylesheet()
{
    // stylesheet template file
    QFile file(QS(":/res/styles/style-template.qss"));
    if (file.open(QFile::ReadOnly)) {
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
        QFont        fnt = QGuiApplication::font();
        QFontMetrics fm(fnt);

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
        styleSheet.replace(QS("%font_small%"), QString::number(font_small) + QSV("pt"));
        styleSheet.replace(QS("%font_large%"), QString::number(font_large) + QSV("pt"));
        styleSheet.replace(QS("%button_height%"), QString::number(button_height) + QSV("px"));
        styleSheet.replace(QS("%top_panel_height%"), QString::number(top_panel_height) + QSV("px"));
        styleSheet.replace(QS("%overlay_header_size%"), QString::number(overlay_header_size) + QSV("px"));
        styleSheet.replace(QS("%context_menu_width%"), QString::number(context_menu_width) + QSV("px"));
        styleSheet.replace(QS("%context_menu_button_height%"), QString::number(context_menu_button_height) + QSV("px"));
        styleSheet.replace(QS("%rename_overlay_width%"), QString::number(rename_overlay_width) + QSV("px"));

        styleSheet.replace(QS("%icontheme%"), QS("light"));
        // Qt::Popup can't do transparency under windows, use square window
#ifdef Q_OS_WIN32
        styleSheet.replace(QS("%contextmenu_border_radius%"), QS("0px"));
#else
        styleSheet.replace(QS("%contextmenu_border_radius%"), QS("3px"));
#endif
        styleSheet.replace(QS("%sys_window%"), sys_window.name());
        styleSheet.replace(QS("%sys_window_tinted%"), sys_window_tinted.name());
        styleSheet.replace(QS("%sys_window_tinted_lc%"), sys_window_tinted_lc.name());
        styleSheet.replace(QS("%sys_window_tinted_lc2%"), sys_window_tinted_lc2.name());
        styleSheet.replace(QS("%sys_window_tinted_hc%"), sys_window_tinted_hc.name());
        styleSheet.replace(QS("%sys_window_tinted_hc2%"), sys_window_tinted_hc2.name());
        styleSheet.replace(QS("%sys_text_secondary_rgba%"), QSV("rgba(") + QString::number(sys_text.red()) + u',' +
                                                            QString::number(sys_text.green()) + u',' +
                                                            QString::number(sys_text.blue()) + QSV(",50%)"));

        styleSheet.replace(QS("%button%"), colors.button.name());
        styleSheet.replace(QS("%button_hover%"), colors.button_hover.name());
        styleSheet.replace(QS("%button_pressed%"), colors.button_pressed.name());
        styleSheet.replace(QS("%panel_button%"), colors.panel_button.name());
        styleSheet.replace(QS("%panel_button_hover%"), colors.panel_button_hover.name());
        styleSheet.replace(QS("%panel_button_pressed%"), colors.panel_button_pressed.name());
        styleSheet.replace(QS("%widget%"), colors.widget.name());
        styleSheet.replace(QS("%widget_border%"), colors.widget_border.name());
        styleSheet.replace(QS("%folderview%"), colors.folderview.name());
        styleSheet.replace(QS("%folderview_topbar%"), colors.folderview_topbar.name());
        styleSheet.replace(QS("%folderview_hc%"), colors.folderview_hc.name());
        styleSheet.replace(QS("%folderview_hc2%"), colors.folderview_hc2.name());
        styleSheet.replace(QS("%accent%"), colors.accent.name());
        styleSheet.replace(QS("%input_field_focus%"), colors.input_field_focus.name());
        styleSheet.replace(QS("%overlay%"), colors.overlay.name());
        styleSheet.replace(QS("%icons%"), colors.icons.name());
        styleSheet.replace(QS("%text_hc2%"), colors.text_hc2.name());
        styleSheet.replace(QS("%text_hc%"), colors.text_hc.name());
        styleSheet.replace(QS("%text%"), colors.text.name());
        styleSheet.replace(QS("%overlay_text%"), colors.overlay_text.name());
        styleSheet.replace(QS("%text_lc%"), colors.text_lc.name());
        styleSheet.replace(QS("%text_lc2%"), colors.text_lc2.name());
        styleSheet.replace(QS("%scrollbar%"), colors.scrollbar.name());
        styleSheet.replace(QS("%scrollbar_hover%"), colors.scrollbar_hover.name());
        styleSheet.replace(QS("%folderview_button_hover%"), colors.folderview_button_hover.name());
        styleSheet.replace(QS("%folderview_button_pressed%"), colors.folderview_button_pressed.name());

        styleSheet.replace(QS("%text_secondary_rgba%"), QSV("rgba(") + QString::number(colors.text.red()) + u',' +
                                                        QString::number(colors.text.green()) + u',' +
                                                        QString::number(colors.text.blue()) + QSV(",62%)"));
        styleSheet.replace(QS("%accent_hover_rgba%"), QSV("rgba(") + QString::number(colors.accent.red()) + u',' +
                                                      QString::number(colors.accent.green()) + u',' +
                                                      QString::number(colors.accent.blue()) + QSV(",65%)"));
        styleSheet.replace(QS("%overlay_rgba%"), QSV("rgba(") + QString::number(colors.overlay.red()) + u',' +
                                                 QString::number(colors.overlay.green()) + u',' +
                                                 QString::number(colors.overlay.blue()) + QSV(",90%)"));
        styleSheet.replace(QS("%fv_backdrop_rgba%"), QSV("rgba(") + QString::number(colors.folderview_hc2.red()) + u',' +
                                                     QString::number(colors.folderview_hc2.green()) + u',' +
                                                     QString::number(colors.folderview_hc2.blue()) + QSV(",80%)"));
        // do not show separator line if topbar color matches folderview
        if (colors.folderview != colors.folderview_topbar)
            styleSheet.replace(QS("%topbar_border_rgba%"), QS("rgba(0,0,0,14%)"));
        else
            styleSheet.replace(QS("%topbar_border_rgba%"), colors.folderview.name());

        // --- apply -------------------------------------------------
        qApp->setStyleSheet(styleSheet);
    }
}
//------------------------------------------------------------------------------
void Settings::loadTheme()
{
    if (useSystemColorScheme()) {
        setColorScheme(ThemeStore::colorScheme(ColorSchemes::SYSTEM));
    } else {
        BaseColorScheme base;
        themeConf.beginGroup(QSV("Colors"));
        base.background            = QColor(themeConf.value(QSV("background"), QS("#1a1a1a")).toString());
        base.background_fullscreen = QColor(themeConf.value(QSV("background_fullscreen"), QS("#1a1a1a")).toString());
        base.text                  = QColor(themeConf.value(QSV("text"), QS("#b6b6b6")).toString());
        base.icons                 = QColor(themeConf.value(QSV("icons"), QS("#a4a4a4")).toString());
        base.widget                = QColor(themeConf.value(QSV("widget"), QS("#252525")).toString());
        base.widget_border         = QColor(themeConf.value(QSV("widget_border"), QS("#2c2c2c")).toString());
        base.accent                = QColor(themeConf.value(QSV("accent"), QS("#8c9b81")).toString());
        base.folderview            = QColor(themeConf.value(QSV("folderview"), QS("#242424")).toString());
        base.folderview_topbar     = QColor(themeConf.value(QSV("folderview_topbar"), QS("#383838")).toString());
        base.scrollbar             = QColor(themeConf.value(QSV("scrollbar"), QS("#5a5a5a")).toString());
        base.overlay_text          = QColor(themeConf.value(QSV("overlay_text"), QS("#d2d2d2")).toString());
        base.overlay               = QColor(themeConf.value(QSV("overlay"), QS("#1a1a1a")).toString());
        base.tid                   = themeConf.value(QSV("tid"), QS("-1")).toInt();
        themeConf.endGroup();
        setColorScheme(ColorScheme(base));
    }
}
void Settings::saveTheme()
{
    if (useSystemColorScheme())
        return;
    themeConf.beginGroup(QSV("Colors"));
    themeConf.setValue(QSV("background"), mColorScheme.background.name());
    themeConf.setValue(QSV("background_fullscreen"), mColorScheme.background_fullscreen.name());
    themeConf.setValue(QSV("text"), mColorScheme.text.name());
    themeConf.setValue(QSV("icons"), mColorScheme.icons.name());
    themeConf.setValue(QSV("widget"), mColorScheme.widget.name());
    themeConf.setValue(QSV("widget_border"), mColorScheme.widget_border.name());
    themeConf.setValue(QSV("accent"), mColorScheme.accent.name());
    themeConf.setValue(QSV("folderview"), mColorScheme.folderview.name());
    themeConf.setValue(QSV("folderview_topbar"), mColorScheme.folderview_topbar.name());
    themeConf.setValue(QSV("scrollbar"), mColorScheme.scrollbar.name());
    themeConf.setValue(QSV("overlay_text"), mColorScheme.overlay_text.name());
    themeConf.setValue(QSV("overlay"), mColorScheme.overlay.name());
    themeConf.setValue(QSV("tid"), mColorScheme.tid);
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
    mVideoFormatsMap.insert("video/webm",       "webm");
    mVideoFormatsMap.insert("video/mp4",        "mp4");
    mVideoFormatsMap.insert("video/mp4",        "m4v");
    mVideoFormatsMap.insert("video/mpeg",       "mpg");
    mVideoFormatsMap.insert("video/mpeg",       "mpeg");
    mVideoFormatsMap.insert("video/x-matroska", "mkv");
    mVideoFormatsMap.insert("video/x-ms-wmv", "wmv");
    mVideoFormatsMap.insert("video/x-msvideo", "avi");
    mVideoFormatsMap.insert("video/quicktime", "mov");
    mVideoFormatsMap.insert("video/x-flv", "flv");
}
//------------------------------------------------------------------------------
QString Settings::mpvBinary() const
{
    QString mpvPath = settingsConf.value(QSV("mpvBinary"), QS("")).toString();
    if (!QFile::exists(mpvPath)) {
#ifdef Q_OS_WIN32
        mpvPath = QCoreApplication::applicationDirPath() + QSV("/mpv.exe");
#elif defined __linux__
        mpvPath = QS("/usr/bin/mpv");
#endif
        if (!QFile::exists(mpvPath))
            mpvPath = QS("");
    }
    return mpvPath;
}

void Settings::setMpvBinary(QString const &path)
{
    if (QFile::exists(path))
        settingsConf.setValue(QSV("mpvBinary"), path);
}
//------------------------------------------------------------------------------
QList<QByteArray> Settings::supportedFormats() const
{
    auto formats = QImageReader::supportedImageFormats();
    formats << "jfif";
    if (videoPlayback())
        formats << mVideoFormatsMap.values();
    formats.removeAll("pdf");
    return formats;
}
//------------------------------------------------------------------------------
// (for open/save dialogs, as a single string)
// example:  QS("Images (*.jpg, *.png)")
QString Settings::supportedFormatsFilter() const
{
    QString filters;
    auto    formats = supportedFormats();
    filters.append(QSV("Supported files ("));
    for (auto &fmt : formats)
        filters.append(QSV("*.") + QString::fromUtf8(fmt) + u' ');
    filters.append(u')');
    return filters;
}
//------------------------------------------------------------------------------
QString Settings::supportedFormatsRegex() const
{
    QString           filter;
    QList<QByteArray> formats = supportedFormats();
    filter.append(QSV(".*\\.("));
    for (auto &fmt : formats)
        filter.append(QString::fromUtf8(fmt) + u'|');
    filter.chop(1);
    filter.append(QSV(")$"));
    return filter;
}
//------------------------------------------------------------------------------
// returns list of mime types
QStringList Settings::supportedMimeTypes() const
{
    QStringList       filters;
    QList<QByteArray> mimeTypes = QImageReader::supportedMimeTypes();
    if (videoPlayback())
        mimeTypes << mVideoFormatsMap.keys();
    for (int i = 0; i < mimeTypes.count(); i++)
        filters << QString::fromLatin1(mimeTypes.at(i));
    return filters;
}
//------------------------------------------------------------------------------
bool Settings::videoPlayback() const
{
#ifdef USE_MPV
    return settingsConf.value(QSV("videoPlayback"), true).toBool();
#else
    return false;
#endif
}

void Settings::setVideoPlayback(bool mode)
{
    settingsConf.setValue(QSV("videoPlayback"), mode);
}
//------------------------------------------------------------------------------
bool Settings::useSystemColorScheme() const
{
    return settingsConf.value(QSV("useSystemColorScheme"), false).toBool();
}

void Settings::setUseSystemColorScheme(bool mode)
{
    settingsConf.setValue(QSV("useSystemColorScheme"), mode);
}
//------------------------------------------------------------------------------
QVersionNumber Settings::lastVersion() const
{
    int vmajor = settingsConf.value(QSV("lastVerMajor"), 0).toInt();
    int vminor = settingsConf.value(QSV("lastVerMinor"), 0).toInt();
    int vmicro = settingsConf.value(QSV("lastVerMicro"), 0).toInt();
    return QVersionNumber(vmajor, vminor, vmicro);
}

void Settings::setLastVersion(QVersionNumber const &ver)
{
    settingsConf.setValue(QSV("lastVerMajor"), ver.majorVersion());
    settingsConf.setValue(QSV("lastVerMinor"), ver.minorVersion());
    settingsConf.setValue(QSV("lastVerMicro"), ver.microVersion());
}
//------------------------------------------------------------------------------
void Settings::setShowChangelogs(bool mode)
{
    settingsConf.setValue(QSV("showChangelogs"), mode);
}

bool Settings::showChangelogs() const
{
    return settingsConf.value(QSV("showChangelogs"), true).toBool();
}
//------------------------------------------------------------------------------
qreal Settings::backgroundOpacity() const
{
    bool  ok    = false;
    qreal value = settingsConf.value(QSV("backgroundOpacity"), 1.0).toReal(&ok);
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
    settingsConf.setValue(QSV("backgroundOpacity"), value);
}
//------------------------------------------------------------------------------
bool Settings::blurBackground() const
{
#ifdef USE_KDE_BLUR
    return settingsConf.value(QSV("blurBackground"), true).toBool();
#else
    return false;
#endif
}

void Settings::setBlurBackground(bool mode)
{
    settingsConf.setValue(QSV("blurBackground"), mode);
}
//------------------------------------------------------------------------------
void Settings::setSortingMode(SortingMode mode)
{
    if (mode > SortingMode::TIME_DESC)
        mode = SortingMode::NAME;
    settingsConf.setValue(QSV("sortingMode"), static_cast<int>(mode));
}

SortingMode Settings::sortingMode() const
{
    int mode = settingsConf.value(QSV("sortingMode"), 0).toInt();
    if (mode < 0 || mode >= 6)
        mode = 0;
    return static_cast<SortingMode>(mode);
}
//------------------------------------------------------------------------------
bool Settings::playVideoSounds() const
{
    return settingsConf.value(QSV("playVideoSounds"), false).toBool();
}

void Settings::setPlayVideoSounds(bool mode)
{
    settingsConf.setValue(QSV("playVideoSounds"), mode);
}
//------------------------------------------------------------------------------
void Settings::setVolume(int vol)
{
    stateConf.setValue(QSV("volume"), vol);
}

int Settings::volume() const
{
    return stateConf.value(QSV("volume"), 100).toInt();
}
//------------------------------------------------------------------------------
FolderViewMode Settings::folderViewMode() const
{
    int mode = settingsConf.value(QSV("folderViewMode"), 2).toInt();
    if (mode < 0 || mode >= 3)
        mode = 2;
    return static_cast<FolderViewMode>(mode);
}

void Settings::setFolderViewMode(FolderViewMode mode)
{
    settingsConf.setValue(QSV("folderViewMode"), static_cast<int>(mode));
}
//------------------------------------------------------------------------------
ThumbPanelStyle Settings::thumbPanelStyle() const
{
    int mode = settingsConf.value(QSV("thumbPanelStyle"), 1).toInt();
    if (mode < 0 || mode > 1)
        mode = 1;
    return static_cast<ThumbPanelStyle>(mode);
}

void Settings::setThumbPanelStyle(ThumbPanelStyle mode)
{
    settingsConf.setValue(QSV("thumbPanelStyle"), static_cast<int>(mode));
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
    int  size = settingsConf.value(QSV("panelPreviewsSize"), 140).toInt(&ok);
    if (!ok)
        size = 140;
    size = qBound(100, size, 250);
    return size;
}

void Settings::setPanelPreviewsSize(int size)
{
    settingsConf.setValue(QSV("panelPreviewsSize"), size);
}
//------------------------------------------------------------------------------
bool Settings::usePreloader() const
{
    return settingsConf.value(QSV("usePreloader"), true).toBool();
}

void Settings::setUsePreloader(bool mode)
{
    settingsConf.setValue(QSV("usePreloader"), mode);
}
//------------------------------------------------------------------------------
bool Settings::keepFitMode() const
{
    return settingsConf.value(QSV("keepFitMode"), false).toBool();
}

void Settings::setKeepFitMode(bool mode)
{
    settingsConf.setValue(QSV("keepFitMode"), mode);
}
//------------------------------------------------------------------------------
bool Settings::fullscreenMode() const
{
    return settingsConf.value(QSV("openInFullscreen"), false).toBool();
}

void Settings::setFullscreenMode(bool mode)
{
    settingsConf.setValue(QSV("openInFullscreen"), mode);
}
//------------------------------------------------------------------------------
bool Settings::maximizedWindow() const
{
    return stateConf.value(QSV("maximizedWindow"), false).toBool();
}

void Settings::setMaximizedWindow(bool mode)
{
    stateConf.setValue(QSV("maximizedWindow"), mode);
}
//------------------------------------------------------------------------------
bool Settings::panelEnabled() const
{
    return settingsConf.value(QSV("panelEnabled"), true).toBool();
}

void Settings::setPanelEnabled(bool mode)
{
    settingsConf.setValue(QSV("panelEnabled"), mode);
}
//------------------------------------------------------------------------------
bool Settings::panelFullscreenOnly() const
{
    return settingsConf.value(QSV("panelFullscreenOnly"), true).toBool();
}

void Settings::setPanelFullscreenOnly(bool mode)
{
    settingsConf.setValue(QSV("panelFullscreenOnly"), mode);
}
//------------------------------------------------------------------------------
int Settings::lastDisplay() const
{
    return stateConf.value(QSV("lastDisplay"), 0).toInt();
}

void Settings::setLastDisplay(int display)
{
    stateConf.setValue(QSV("lastDisplay"), display);
}
//------------------------------------------------------------------------------
PanelPosition Settings::panelPosition() const
{
    QString posString = settingsConf.value(QSV("panelPosition"), QS("top")).toString();
    if (posString == QSV("top"))
        return PanelPosition::TOP;
    else if (posString == QSV("bottom"))
        return PanelPosition::BOTTOM;
    else if (posString == QSV("left"))
        return PanelPosition::LEFT;
    else
        return PanelPosition::RIGHT;
}

void Settings::setPanelPosition(PanelPosition pos)
{
    QString posString;
    switch (pos) {
    case PanelPosition::TOP:
        posString = QS("top");
        break;
    case PanelPosition::BOTTOM:
        posString = QS("bottom");
        break;
    case PanelPosition::LEFT:
        posString = QS("left");
        break;
    case PanelPosition::RIGHT:
        posString = QS("right");
        break;
    }
    settingsConf.setValue(QSV("panelPosition"), posString);
}
//------------------------------------------------------------------------------
bool Settings::panelPinned() const
{
    return settingsConf.value(QSV("panelPinned"), false).toBool();
}

void Settings::setPanelPinned(bool mode)
{
    settingsConf.setValue(QSV("panelPinned"), mode);
}
//------------------------------------------------------------------------------
/*
 * 0: fit window
 * 1: fit width
 * 2: orginal size
 */
ImageFitMode Settings::imageFitMode() const
{
    int mode = settingsConf.value(QSV("defaultFitMode"), 0).toInt();
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
    settingsConf.setValue(QSV("defaultFitMode"), modeInt);
}
//------------------------------------------------------------------------------
QRect Settings::windowGeometry() const
{
    QRect savedRect = stateConf.value(QSV("windowGeometry")).toRect();
    if (savedRect.size().isEmpty())
        savedRect.setRect(100, 100, 900, 600);
    return savedRect;
}

void Settings::setWindowGeometry(QRect geometry)
{
    stateConf.setValue(QSV("windowGeometry"), geometry);
}
//------------------------------------------------------------------------------
bool Settings::loopSlideshow() const
{
    return settingsConf.value(QSV("loopSlideshow"), false).toBool();
}

void Settings::setLoopSlideshow(bool mode)
{
    settingsConf.setValue(QSV("loopSlideshow"), mode);
}
//------------------------------------------------------------------------------
void Settings::sendChangeNotification()
{
    emit settingsChanged();
}
//------------------------------------------------------------------------------
void Settings::readShortcuts(QMap<QString, QString> &shortcuts)
{
    settingsConf.beginGroup(QS("Controls"));
    QStringList in, pair;
    in = settingsConf.value(QSV("shortcuts")).toStringList();
    for (int i = 0; i < in.count(); i++) {
        pair = in[i].split(u'=');
        if (!pair[0].isEmpty() && !pair[1].isEmpty()) {
            if (pair[1] == QSV("eq"))
                pair[1] = u'=';
            shortcuts.insert(pair[1], pair[0]);
        }
    }
    settingsConf.endGroup();
}

void Settings::saveShortcuts(QMap<QString, QString> const &shortcuts)
{
    settingsConf.beginGroup(QSV("Controls"));
    QMapIterator<QString, QString> i(shortcuts);
    QStringList                    out;
    while (i.hasNext()) {
        i.next();
        if (i.key() == u'=')
            out << i.value() + u'=' + QS("eq");
        else
            out << i.value() + u'=' + i.key();
    }
    settingsConf.setValue(QSV("shortcuts"), out);
    settingsConf.endGroup();
}
//------------------------------------------------------------------------------
void Settings::readScripts(QMap<QString, Script> &scripts)
{
    scripts.clear();
    settingsConf.beginGroup(QSV("Scripts"));
    int size = settingsConf.beginReadArray(QSV("script"));
    for (int i = 0; i < size; i++) {
        settingsConf.setArrayIndex(i);
        QString  name  = settingsConf.value(QSV("name")).toString();
        QVariant value = settingsConf.value(QSV("value"));
        Script   scr   = value.value<Script>();
        scripts.insert(name, scr);
    }
    settingsConf.endArray();
    settingsConf.endGroup();
}

void Settings::saveScripts(QMap<QString, Script> const &scripts)
{
    settingsConf.beginGroup(QSV("Scripts"));
    settingsConf.beginWriteArray(QSV("script"));
    QMapIterator i(scripts);
    unsigned     counter = 0;
    while (i.hasNext()) {
        i.next();
        settingsConf.setArrayIndex(counter);
        settingsConf.setValue(QSV("name"), i.key());
        settingsConf.setValue(QSV("value"), QVariant::fromValue(i.value()));
        counter++;
    }
    settingsConf.endArray();
    settingsConf.endGroup();
}
//------------------------------------------------------------------------------
bool Settings::squareThumbnails() const
{
    return settingsConf.value(QSV("squareThumbnails"), false).toBool();
}

void Settings::setSquareThumbnails(bool mode)
{
    settingsConf.setValue(QSV("squareThumbnails"), mode);
}
//------------------------------------------------------------------------------
bool Settings::transparencyGrid() const
{
    return settingsConf.value(QSV("drawTransparencyGrid"), false).toBool();
}

void Settings::setTransparencyGrid(bool mode)
{
    settingsConf.setValue(QSV("drawTransparencyGrid"), mode);
}
//------------------------------------------------------------------------------
bool Settings::enableSmoothScroll() const
{
    return settingsConf.value(QSV("enableSmoothScroll"), true).toBool();
}

void Settings::setEnableSmoothScroll(bool mode)
{
    settingsConf.setValue(QSV("enableSmoothScroll"), mode);
}
//------------------------------------------------------------------------------
bool Settings::useThumbnailCache() const
{
    return settingsConf.value(QSV("thumbnailCache"), true).toBool();
}

void Settings::setUseThumbnailCache(bool mode)
{
    settingsConf.setValue(QSV("thumbnailCache"), mode);
}
//------------------------------------------------------------------------------
QStringList Settings::savedPaths() const
{
    return stateConf.value(QSV("savedPaths"), QDir::homePath()).toStringList();
}

void Settings::setSavedPaths(QStringList const &paths)
{
    stateConf.setValue(QSV("savedPaths"), paths);
}
//------------------------------------------------------------------------------
QStringList Settings::bookmarks() const
{
    return stateConf.value(QSV("bookmarks")).toStringList();
}

void Settings::setBookmarks(QStringList const &paths)
{
    stateConf.setValue(QSV("bookmarks"), paths);
}
//------------------------------------------------------------------------------
bool Settings::placesPanel() const
{
    return stateConf.value(QSV("placesPanel"), true).toBool();
}

void Settings::setPlacesPanel(bool mode)
{
    stateConf.setValue(QSV("placesPanel"), mode);
}
//------------------------------------------------------------------------------
bool Settings::placesPanelBookmarksExpanded() const
{
    return stateConf.value(QSV("placesPanelBookmarksExpanded"), true).toBool();
}

void Settings::setPlacesPanelBookmarksExpanded(bool mode)
{
    stateConf.setValue(QSV("placesPanelBookmarksExpanded"), mode);
}
//------------------------------------------------------------------------------
bool Settings::placesPanelTreeExpanded() const
{
    return stateConf.value(QSV("placesPanelTreeExpanded"), true).toBool();
}

void Settings::setPlacesPanelTreeExpanded(bool mode)
{
    stateConf.setValue(QSV("placesPanelTreeExpanded"), mode);
}
//------------------------------------------------------------------------------
int Settings::placesPanelWidth() const
{
    return stateConf.value(QSV("placesPanelWidth"), 260).toInt();
}

void Settings::setPlacesPanelWidth(int width)
{
    stateConf.setValue(QSV("placesPanelWidth"), width);
}
//------------------------------------------------------------------------------
void Settings::setSlideshowInterval(int ms)
{
    settingsConf.setValue(QSV("slideshowInterval"), ms);
}

int Settings::slideshowInterval() const
{
    int interval = settingsConf.value(QSV("slideshowInterval"), 3000).toInt();
    if (interval <= 0)
        interval = 3000;
    return interval;
}
//------------------------------------------------------------------------------
int Settings::thumbnailerThreadCount() const
{
    int count = settingsConf.value(QSV("thumbnailerThreads"), 4).toInt();
    if (count < 1)
        count = 4;
    return count;
}

void Settings::setThumbnailerThreadCount(int count)
{
    settingsConf.setValue(QSV("thumbnailerThreads"), count);
}
//------------------------------------------------------------------------------
bool Settings::smoothUpscaling() const
{
    return settingsConf.value(QSV("smoothUpscaling"), true).toBool();
}

void Settings::setSmoothUpscaling(bool mode)
{
    settingsConf.setValue(QSV("smoothUpscaling"), mode);
}
//------------------------------------------------------------------------------
int Settings::folderViewIconSize() const
{
    return settingsConf.value(QSV("folderViewIconSize"), 120).toInt();
}

void Settings::setFolderViewIconSize(int value)
{
    settingsConf.setValue(QSV("folderViewIconSize"), value);
}
//------------------------------------------------------------------------------
bool Settings::expandImage() const
{
    return settingsConf.value(QSV("expandImage"), false).toBool();
}

void Settings::setExpandImage(bool mode)
{
    settingsConf.setValue(QSV("expandImage"), mode);
}
//------------------------------------------------------------------------------
int Settings::expandLimit() const
{
    return settingsConf.value(QSV("expandLimit"), 2).toInt();
}

void Settings::setExpandLimit(int value)
{
    settingsConf.setValue(QSV("expandLimit"), value);
}
//------------------------------------------------------------------------------
int Settings::JPEGSaveQuality() const
{
    int quality = std::clamp(settingsConf.value(QSV("JPEGSaveQuality"), 95).toInt(), 0, 100);
    return quality;
}

void Settings::setJPEGSaveQuality(int value)
{
    settingsConf.setValue(QSV("JPEGSaveQuality"), value);
}
//------------------------------------------------------------------------------
ScalingFilter Settings::scalingFilter() const
{
    int defaultFilter = 1;
#ifdef USE_OPENCV
    // default to a nicer CV_CUBIC
    defaultFilter = 3;
#endif
    int mode = settingsConf.value(QSV("scalingFilter"), defaultFilter).toInt();
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
    settingsConf.setValue(QSV("scalingFilter"), static_cast<int>(mode));
}
//------------------------------------------------------------------------------
bool Settings::smoothAnimatedImages() const
{
    return settingsConf.value(QSV("smoothAnimatedImages"), true).toBool();
}

void Settings::setSmoothAnimatedImages(bool mode)
{
    settingsConf.setValue(QSV("smoothAnimatedImages"), mode);
}
//------------------------------------------------------------------------------
bool Settings::infoBarFullscreen() const
{
    return settingsConf.value(QSV("infoBarFullscreen"), true).toBool();
}

void Settings::setInfoBarFullscreen(bool mode)
{
    settingsConf.setValue(QSV("infoBarFullscreen"), mode);
}
//------------------------------------------------------------------------------
bool Settings::infoBarWindowed() const
{
    return settingsConf.value(QSV("infoBarWindowed"), false).toBool();
}

void Settings::setInfoBarWindowed(bool mode)
{
    settingsConf.setValue(QSV("infoBarWindowed"), mode);
}
//------------------------------------------------------------------------------
bool Settings::windowTitleExtendedInfo() const
{
    return settingsConf.value(QSV("windowTitleExtendedInfo"), true).toBool();
}

void Settings::setWindowTitleExtendedInfo(bool mode)
{
    settingsConf.setValue(QSV("windowTitleExtendedInfo"), mode);
}

//------------------------------------------------------------------------------
bool Settings::cursorAutohide() const
{
    return settingsConf.value(QSV("cursorAutohiding"), true).toBool();
}

void Settings::setCursorAutohide(bool mode)
{
    settingsConf.setValue(QSV("cursorAutohiding"), mode);
}
//------------------------------------------------------------------------------
bool Settings::firstRun() const
{
    return settingsConf.value(QSV("firstRun"), true).toBool();
}

void Settings::setFirstRun(bool mode)
{
    settingsConf.setValue(QSV("firstRun"), mode);
}
//------------------------------------------------------------------------------
bool Settings::showSaveOverlay() const
{
    return settingsConf.value(QSV("showSaveOverlay"), true).toBool();
}

void Settings::setShowSaveOverlay(bool mode)
{
    settingsConf.setValue(QSV("showSaveOverlay"), mode);
}
//------------------------------------------------------------------------------
bool Settings::confirmDelete() const
{
    return settingsConf.value(QSV("confirmDelete"), true).toBool();
}

void Settings::setConfirmDelete(bool mode)
{
    settingsConf.setValue(QSV("confirmDelete"), mode);
}
//------------------------------------------------------------------------------
bool Settings::confirmTrash() const
{
    return settingsConf.value(QSV("confirmTrash"), true).toBool();
}

void Settings::setConfirmTrash(bool mode)
{
    settingsConf.setValue(QSV("confirmTrash"), mode);
}
//------------------------------------------------------------------------------
bool Settings::unloadThumbs() const
{
    return settingsConf.value(QSV("unloadThumbs"), true).toBool();
}

void Settings::setUnloadThumbs(bool mode)
{
    settingsConf.setValue(QSV("unloadThumbs"), mode);
}
//------------------------------------------------------------------------------
float Settings::zoomStep() const
{
    bool  ok    = false;
    float value = settingsConf.value(QSV("zoomStep"), 0.2f).toFloat(&ok);
    if (!ok)
        return 0.2f;
    value = qBound(0.01f, value, 0.5f);
    return value;
}

void Settings::setZoomStep(float value)
{
    value = qBound(0.01f, value, 0.5f);
    settingsConf.setValue(QSV("zoomStep"), value);
}
//------------------------------------------------------------------------------
void Settings::setZoomIndicatorMode(ZoomIndicatorMode mode)
{
    settingsConf.setValue(QSV("zoomIndicatorMode"), static_cast<int>(mode));
}

ZoomIndicatorMode Settings::zoomIndicatorMode() const
{
    int mode = settingsConf.value(QSV("zoomIndicatorMode"), 0).toInt();
    if (mode < 0 || mode > 2)
        mode = 0;
    return static_cast<ZoomIndicatorMode>(mode);
}
//------------------------------------------------------------------------------
void Settings::setFocusPointIn1to1Mode(ImageFocusPoint mode)
{
    settingsConf.setValue(QSV("focusPointIn1to1Mode"), static_cast<int>(mode));
}

ImageFocusPoint Settings::focusPointIn1to1Mode() const
{
    int mode = settingsConf.value(QSV("focusPointIn1to1Mode"), 1).toInt();
    if (mode < 0 || mode > 2)
        mode = 1;
    return static_cast<ImageFocusPoint>(mode);
}

void Settings::setDefaultCropAction(DefaultCropAction mode)
{
    settingsConf.setValue(QSV("defaultCropAction"), static_cast<int>(mode));
}

DefaultCropAction Settings::defaultCropAction() const
{
    int mode = settingsConf.value(QSV("defaultCropAction"), 0).toInt();
    if (mode < 0 || mode > 1)
        mode = 0;
    return static_cast<DefaultCropAction>(mode);
}

ImageScrolling Settings::imageScrolling() const
{
    int mode = settingsConf.value(QSV("imageScrolling"), 1).toInt();
    if (mode < 0 || mode > 2)
        mode = 0;
    return static_cast<ImageScrolling>(mode);
}

void Settings::setImageScrolling(ImageScrolling mode)
{
    settingsConf.setValue(QSV("imageScrolling"), static_cast<int>(mode));
}
//------------------------------------------------------------------------------
ViewMode Settings::defaultViewMode() const
{
    int mode = settingsConf.value(QSV("defaultViewMode"), 0).toInt();
    if (mode < 0 || mode > 1)
        mode = 0;
    return static_cast<ViewMode>(mode);
}

void Settings::setDefaultViewMode(ViewMode mode)
{
    settingsConf.setValue(QSV("defaultViewMode"), static_cast<int>(mode));
}
//------------------------------------------------------------------------------
FolderEndAction Settings::folderEndAction() const
{
    int mode = settingsConf.value(QSV("folderEndAction"), 0).toInt();
    if (mode < 0 || mode > 2)
        mode = 0;
    return static_cast<FolderEndAction>(mode);
}

void Settings::setFolderEndAction(FolderEndAction mode)
{
    settingsConf.setValue(QSV("folderEndAction"), static_cast<int>(mode));
}
//------------------------------------------------------------------------------
bool Settings::printLandscape() const
{
    return stateConf.value(QSV("printLandscape"), false).toBool();
}

void Settings::setPrintLandscape(bool mode)
{
    stateConf.setValue(QSV("printLandscape"), mode);
}
//------------------------------------------------------------------------------
bool Settings::printPdfDefault() const
{
    return stateConf.value(QSV("printPdfDefault"), false).toBool();
}

void Settings::setPrintPdfDefault(bool mode)
{
    stateConf.setValue(QSV("printPdfDefault"), mode);
}
//------------------------------------------------------------------------------
bool Settings::printColor() const
{
    return stateConf.value(QSV("printColor"), false).toBool();
}

void Settings::setPrintColor(bool mode)
{
    stateConf.setValue(QSV("printColor"), mode);
}
//------------------------------------------------------------------------------
bool Settings::printFitToPage() const
{
    return stateConf.value(QSV("printFitToPage"), true).toBool();
}

void Settings::setPrintFitToPage(bool mode)
{
    stateConf.setValue(QSV("printFitToPage"), mode);
}
//------------------------------------------------------------------------------
QString Settings::lastPrinter() const
{
    return stateConf.value(QSV("lastPrinter"), QS("")).toString();
}

void Settings::setLastPrinter(QString const &name)
{
    stateConf.setValue(QSV("lastPrinter"), name);
}
//------------------------------------------------------------------------------
bool Settings::jxlAnimation() const
{
    return settingsConf.value(QSV("jxlAnimation"), false).toBool();
}

void Settings::setJxlAnimation(bool mode)
{
    settingsConf.setValue(QSV("jxlAnimation"), mode);
}
//------------------------------------------------------------------------------
bool Settings::autoResizeWindow() const
{
    return settingsConf.value(QSV("autoResizeWindow"), false).toBool();
}

void Settings::setAutoResizeWindow(bool mode)
{
    settingsConf.setValue(QSV("autoResizeWindow"), mode);
}
//------------------------------------------------------------------------------
int Settings::autoResizeLimit() const
{
    int limit = settingsConf.value(QSV("autoResizeLimit"), 90).toInt();
    if (limit < 30 || limit > 100)
        limit = 90;
    return limit;
}

void Settings::setAutoResizeLimit(int percent)
{
    settingsConf.setValue(QSV("autoResizeLimit"), percent);
}
//------------------------------------------------------------------------------
int Settings::memoryAllocationLimit() const
{
    int limit = settingsConf.value(QSV("memoryAllocationLimit"), 1024).toInt();
    if (limit < 512)
        limit = 512;
    else if (limit > 8192)
        limit = 8192;
    return limit;
}

void Settings::setMemoryAllocationLimit(int limitMB)
{
    settingsConf.setValue(QSV("memoryAllocationLimit"), limitMB);
}
//------------------------------------------------------------------------------
bool Settings::panelCenterSelection() const
{
    return settingsConf.value(QSV("panelCenterSelection"), false).toBool();
}

void Settings::setPanelCenterSelection(bool mode)
{
    settingsConf.setValue(QSV("panelCenterSelection"), mode);
}
//------------------------------------------------------------------------------
QString Settings::language() const
{
    return settingsConf.value(QSV("language"), QS("en_US")).toString();
}

void Settings::setLanguage(QString const &lang)
{
    settingsConf.setValue(QSV("language"), lang);
}
//------------------------------------------------------------------------------
bool Settings::useFixedZoomLevels() const
{
    return settingsConf.value(QSV("useFixedZoomLevels"), false).toBool();
}

void Settings::setUseFixedZoomLevels(bool mode)
{
    settingsConf.setValue(QSV("useFixedZoomLevels"), mode);
}
//------------------------------------------------------------------------------
QString Settings::defaultZoomLevels()
{
    return QS("0.05,0.1,0.125,0.166,0.25,0.333,0.5,0.66,1,1.5,2,3,4,5,6,7,8");
}
QString Settings::zoomLevels() const
{
    return settingsConf.value(QSV("fixedZoomLevels"), defaultZoomLevels()).toString();
}

void Settings::setZoomLevels(QString const &levels)
{
    settingsConf.setValue(QSV("fixedZoomLevels"), levels);
}
//------------------------------------------------------------------------------
bool Settings::unlockMinZoom() const
{
    return settingsConf.value(QSV("unlockMinZoom"), true).toBool();
}

void Settings::setUnlockMinZoom(bool mode)
{
    settingsConf.setValue(QSV("unlockMinZoom"), mode);
}
//------------------------------------------------------------------------------
bool Settings::sortFolders() const
{
    return settingsConf.value(QSV("sortFolders"), true).toBool();
}

void Settings::setSortFolders(bool mode)
{
    settingsConf.setValue(QSV("sortFolders"), mode);
}
//------------------------------------------------------------------------------
bool Settings::trackpadDetection() const
{
    return settingsConf.value(QSV("trackpadDetection"), true).toBool();
}

void Settings::setTrackpadDetection(bool mode)
{
    settingsConf.setValue(QSV("trackpadDetection"), mode);
}
