#include "VideoPlayerInitProxy.h"

#ifdef Q_OS_WINDOWS
# include "Platform.h"
# define QStrToWChar(s) (reinterpret_cast<wchar_t *>(s.data()))
#endif
#ifdef _QIMGV_PLAYER_PLUGIN
# define QIMGV_PLAYER_PLUGIN _QIMGV_PLAYER_PLUGIN
#else
# define QIMGV_PLAYER_PLUGIN ""
#endif

VideoPlayerInitProxy::VideoPlayerInitProxy(QWidget *parent)
    : VideoPlayer(parent),
      player(nullptr),
      layout(new QVBoxLayout(this)),
      errorLabel(nullptr),
      libFile(QStringLiteral(QIMGV_PLAYER_PLUGIN))
{
    setAccessibleName(u"VideoPlayerInitProxy"_s);
    setMouseTracking(true);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    connect(settings, &Settings::settingsChanged, this, &VideoPlayerInitProxy::onSettingsChanged);

#ifdef Q_OS_WIN32
    libDirs << QApplication::applicationDirPath() + u"/plugins";
#else
    QDir libPath(QApplication::applicationDirPath() + u"/../lib/qimgv"_s);
    libDirs << (libPath.makeAbsolute() ? libPath.path() : u"."_s) << u"/usr/lib/qimgv"_s << u"/usr/lib64/qimgv"_s;
#endif
}

VideoPlayerInitProxy::~VideoPlayerInitProxy()
{
#ifdef Q_OS_WINDOWS
    if (hPlayerModule)
        ::FreeLibrary(hPlayerModule);
#endif
    if (playerLib.isLoaded())
        playerLib.unload();
}

void VideoPlayerInitProxy::onSettingsChanged()
{
    if (!player)
        return;
    player->setMuted(!settings->playVideoSounds());
    player->setVideoUnscaled(!settings->expandImage());
}

QSharedPointer<VideoPlayer> VideoPlayerInitProxy::getPlayer()
{
    return player;
}

bool VideoPlayerInitProxy::isInitialized() const
{
    return player != nullptr;
}

inline bool VideoPlayerInitProxy::initPlayer()
{
#ifdef USE_MPV
    if (player)
        return true;

    QFileInfo pluginFile;
    for (auto const &dir : libDirs) {
        pluginFile.setFile(dir + u'/' + libFile);
        if (pluginFile.isFile() && pluginFile.isReadable()) {
            playerLib.setFileName(pluginFile.absoluteFilePath());
            break;
        }
    }
    if (playerLib.fileName().isEmpty()) {
        qWarning() << u"Could not find" << libFile << u"in the following directories:" << libDirs;
        return false;
    }

#if defined Q_OS_WINDOWS
    QString mpvDll  = QDir::toNativeSeparators(pluginFile.absolutePath()) + u'\\' + u"libmpv-2.dll";
    HMODULE hLibMpv = ::LoadLibraryExW(reinterpret_cast<wchar_t *>(mpvDll.data()), nullptr, 0);
    if (!hLibMpv) {
        qWarning() << u"Error loading library libmpv";
        return false;
    }
#endif
    if (!playerLib.load()) {
        qWarning() << u"Error loading library:" << playerLib.errorString();
        return false;
    }
#ifdef Q_OS_WINDOWS
    ::FreeLibrary(hLibMpv);
#endif

    using CreatePlayerWidget_t = VideoPlayer *(*)(QWidget *);
    //NOLINTNEXTLINE(clang-diagnostic-cast-function-type-strict)
    auto CreatePlayerWidget = reinterpret_cast<CreatePlayerWidget_t>(playerLib.resolve("CreatePlayerWidget"));

    if (!CreatePlayerWidget) {
        qWarning() << u"Error resolving function:" << playerLib.errorString();
        return false;
    }
    player.reset(CreatePlayerWidget(this));
    if (!player) {
        qWarning() << u"Could not load:" << playerLib.fileName() << u". Wrong plugin version?";
        return false;
    }

    player->setMuted(!settings->playVideoSounds());
    player->setVideoUnscaled(!settings->expandImage());
    player->setVolume(settings->volume());
    player->setParent(this);
    layout->addWidget(player.get());
    player->hide();
    setFocusProxy(player.get());

    connect(player.get(), &VideoPlayer::durationChanged,  this, &VideoPlayerInitProxy::durationChanged);
    connect(player.get(), &VideoPlayer::positionChanged,  this, &VideoPlayerInitProxy::positionChanged);
    connect(player.get(), &VideoPlayer::videoPaused,      this, &VideoPlayerInitProxy::videoPaused);
    connect(player.get(), &VideoPlayer::playbackFinished, this, &VideoPlayerInitProxy::playbackFinished);

    return true;
#else
    return false;
#endif
}

bool VideoPlayerInitProxy::showVideo(QString const &file)
{
    if (!initPlayer())
        return false;
    return player->showVideo(file);
}

void VideoPlayerInitProxy::seek(int64_t pos)
{
    if (!player)
        return;
    player->seek(pos);
}

void VideoPlayerInitProxy::seekRelative(int64_t pos)
{
    if (!player)
        return;
    player->seekRelative(pos);
}

void VideoPlayerInitProxy::pauseResume()
{
    if (!player)
        return;
    player->pauseResume();
}

void VideoPlayerInitProxy::frameStep()
{
    if (!player)
        return;
    player->frameStep();
}

void VideoPlayerInitProxy::frameStepBack()
{
    if (!player)
        return;
    player->frameStepBack();
}

void VideoPlayerInitProxy::stop()
{
    if (!player)
        return;
    player->stop();
}

void VideoPlayerInitProxy::setPaused(bool mode)
{
    if (!player)
        return;
    player->setPaused(mode);
}

void VideoPlayerInitProxy::setMuted(bool mode)
{
    if (!player)
        return;
    player->setMuted(mode);
}

bool VideoPlayerInitProxy::muted() const
{
    if (!player)
        return true;
    return player->muted();
}

void VideoPlayerInitProxy::volumeUp()
{
    if (!player)
        return;
    player->volumeUp();
    settings->setVolume(player->volume());
}

void VideoPlayerInitProxy::volumeDown()
{
    if (!player)
        return;
    player->volumeDown();
    settings->setVolume(player->volume());
}

void VideoPlayerInitProxy::setVolume(int vol)
{
    if (!player)
        return;
    player->setVolume(vol);
}

int VideoPlayerInitProxy::volume() const
{
    if (!player)
        return 0;
    return player->volume();
}

void VideoPlayerInitProxy::setVideoUnscaled(bool mode)
{
    if (!player)
        return;
    player->setVideoUnscaled(mode);
}

void VideoPlayerInitProxy::setLoopPlayback(bool mode)
{
    if (!player)
        return;
    player->setLoopPlayback(mode);
}

void VideoPlayerInitProxy::show()
{
    if (initPlayer()) {
        if (errorLabel)
            layout->removeWidget(errorLabel);
        player->show();
    } else if (!errorLabel) {
        errorLabel = new QLabel(this);
        errorLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        errorLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        // errorLabel->setAlignment(Qt::AlignVCenter);
        QString errString = u"Could not load " + libFile + u" from:";
        for (auto const &path : libDirs)
            errString.append(u'\n' + path + u'/');
        errorLabel->setText(errString);
        layout->addWidget(errorLabel);
    }
    VideoPlayer::show();
}

void VideoPlayerInitProxy::hide()
{
    if (player)
        player->hide();
    VideoPlayer::hide();
    player.reset();
}
