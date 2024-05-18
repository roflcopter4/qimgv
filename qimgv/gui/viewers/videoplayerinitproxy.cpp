#include "videoplayerinitproxy.h"

#ifdef Q_OS_WIN32
# include "Windows.h"
#endif

#ifdef _QIMGV_PLAYER_PLUGIN
    #define QIMGV_PLAYER_PLUGIN _QIMGV_PLAYER_PLUGIN
#else
    #define QIMGV_PLAYER_PLUGIN ""
#endif

VideoPlayerInitProxy::VideoPlayerInitProxy(QWidget *parent)
    : VideoPlayer(parent),
      player(nullptr)
{
    setAccessibleName(QS("VideoPlayerInitProxy"));
    setMouseTracking(true);
    layout.setContentsMargins(0, 0, 0, 0);
    setLayout(&layout);
    connect(settings, &Settings::settingsChanged, this, &VideoPlayerInitProxy::onSettingsChanged);
    libFile = QIMGV_PLAYER_PLUGIN;
#ifdef Q_OS_WIN32
    libDirs << QApplication::applicationDirPath() + QS("/plugins");
#else
    QDir libPath(QApplication::applicationDirPath() + QS("/../lib/qimgv"));
    libDirs << (libPath.makeAbsolute() ? libPath.path() : QS(".")) << QS("/usr/lib/qimgv")
            << QS("/usr/lib64/qimgv");
#endif
}

VideoPlayerInitProxy::~VideoPlayerInitProxy()
{}

void VideoPlayerInitProxy::onSettingsChanged()
{
    if (!player)
        return;
    player->setMuted(!settings->playVideoSounds());
    player->setVideoUnscaled(!settings->expandImage());
}

std::shared_ptr<VideoPlayer> VideoPlayerInitProxy::getPlayer()
{
    return player;
}

bool VideoPlayerInitProxy::isInitialized() const
{
    return (player != nullptr);
}

inline bool VideoPlayerInitProxy::initPlayer()
{
#ifndef USE_MPV
    return false;
#endif
    if (player)
        return true;

    QFileInfo pluginFile;
    for (auto const &dir : libDirs) {
        pluginFile.setFile(dir + QS("/") + libFile);
        if (pluginFile.isFile() && pluginFile.isReadable()) {
            playerLib.setFileName(pluginFile.absoluteFilePath());
            break;
        }
    }
    if (playerLib.fileName().isEmpty()) {
        qDebug() << QSV("Could not find") << libFile << QSV("in the following directories:") << libDirs;
        return false;
    }

    using createPlayerWidgetFn = VideoPlayer *(*)();

    playerLib.load();

    // load lib
    if (auto fn = reinterpret_cast<createPlayerWidgetFn>(playerLib.resolve("CreatePlayerWidget"))) {
        VideoPlayer *pl = fn();
        player.reset(pl);
    }
    if (!player) {
        qDebug() << QSV("Could not load:") << playerLib.fileName() << QSV(". Wrong plugin version?");
        return false;
    }

    player->setMuted(!settings->playVideoSounds());
    player->setVideoUnscaled(!settings->expandImage());
    player->setVolume(settings->volume());

    player->setParent(this);
    layout.addWidget(player.get());
    player->hide();
    setFocusProxy(player.get());
    connect(player.get(), SIGNAL(durationChanged(int)), this, SIGNAL(durationChanged(int)));
    connect(player.get(), SIGNAL(positionChanged(int)), this, SIGNAL(positionChanged(int)));
    connect(player.get(), SIGNAL(videoPaused(bool)), this, SIGNAL(videoPaused(bool)));
    connect(player.get(), SIGNAL(playbackFinished()), this, SIGNAL(playbackFinished()));
    return true;
}

bool VideoPlayerInitProxy::showVideo(QString const &file)
{
    if (!initPlayer())
        return false;
    return player->showVideo(file);
}

void VideoPlayerInitProxy::seek(int pos)
{
    if (!player)
        return;
    player->seek(pos);
}

void VideoPlayerInitProxy::seekRelative(int pos)
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
            layout.removeWidget(errorLabel);
        player->show();
    } else if (!errorLabel) {
        errorLabel = new QLabel(this);
        errorLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        errorLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        // errorLabel->setAlignment(Qt::AlignVCenter);
        QString errString = QS("Could not load ") + libFile + QS(" from:");
        for (auto const &path : libDirs)
            errString.append(QS("\n") + path + QS("/"));
        errorLabel->setText(errString);
        layout.addWidget(errorLabel);
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

void VideoPlayerInitProxy::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
}
