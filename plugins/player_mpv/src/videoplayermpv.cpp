#include "videoplayermpv.h"
#include "mpvwidget.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFileDialog>

#define QS(s) QStringLiteral(s)

// TODO: window flashes white when opening a video (straight from file manager)
VideoPlayerMpv::VideoPlayerMpv(QWidget *parent)
    : VideoPlayer(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);

    m_mpv    = new MpvWidget(this);
    auto *vl = new QVBoxLayout();
    vl->setContentsMargins(0,0,0,0);
    vl->addWidget(m_mpv);
    setLayout(vl);

    setFocusPolicy(Qt::NoFocus);
    m_mpv->setFocusPolicy(Qt::NoFocus);

    readSettings();
    //connect(settings, SIGNAL(settingsChanged()), this, SLOT(readSettings()));
    connect(m_mpv, SIGNAL(durationChanged(int)), this, SIGNAL(durationChanged(int)));
    connect(m_mpv, SIGNAL(positionChanged(int)), this, SIGNAL(positionChanged(int)));
    connect(m_mpv, SIGNAL(videoPaused(bool)),    this, SIGNAL(videoPaused(bool)));
    connect(m_mpv, SIGNAL(playbackFinished()),   this, SIGNAL(playbackFinished()));
}

bool VideoPlayerMpv::showVideo(QString const &file)
{
    if (file.isEmpty())
        return false;
    m_mpv->command(QStringList() << QS("loadfile") << file);
    setPaused(false);
    return true;
}

void VideoPlayerMpv::seek(int pos)
{
    m_mpv->command(QVariantList() << QS("seek") << pos << QS("absolute"));
    //qDebug() << "seek(): " << pos << " sec";
}

void VideoPlayerMpv::seekRelative(int pos)
{
    m_mpv->command(QVariantList() << QS("seek") << pos << QS("relative"));
    //qDebug() << "seekRelative(): " << pos << " sec";
}

void VideoPlayerMpv::pauseResume()
{
    bool paused = m_mpv->getProperty(QS("pause")).toBool();
    setPaused(!paused);
}

void VideoPlayerMpv::frameStep()
{
    m_mpv->command(QVariantList() << QS("frame-step"));
}

void VideoPlayerMpv::frameStepBack()
{
    m_mpv->command(QVariantList() << QS("frame-back-step"));
}

void VideoPlayerMpv::stop()
{
    m_mpv->command(QVariantList() << QS("stop"));
}

void VideoPlayerMpv::setPaused(bool mode)
{
    m_mpv->setProperty(QS("pause"), mode);
}

void VideoPlayerMpv::setMuted(bool mode)
{
    m_mpv->setMuted(mode);
}

bool VideoPlayerMpv::muted() const
{
    return m_mpv->muted();
}

void VideoPlayerMpv::volumeUp()
{
    m_mpv->setVolume(m_mpv->volume() + 5);
}

void VideoPlayerMpv::volumeDown()
{
    m_mpv->setVolume(m_mpv->volume() - 5);
}

void VideoPlayerMpv::setVolume(int vol)
{
    m_mpv->setVolume(vol);
}

int VideoPlayerMpv::volume() const
{
    return m_mpv->volume();
}

void VideoPlayerMpv::setVideoUnscaled(bool mode)
{
    if (mode)
        m_mpv->setOption(QS("video-unscaled"), QS("downscale-big"));
    else
        m_mpv->setOption(QS("video-unscaled"), QS("no"));
}

void VideoPlayerMpv::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
}

void VideoPlayerMpv::readSettings()
{
#if 0
    setMuted(!settings->playVideoSounds());
    setVideoUnscaled(!settings->expandImage());
#endif
}

void VideoPlayerMpv::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton &&
        event->type() != QEvent::MouseButtonDblClick)
    {
        event->accept();
        this->pauseResume();
    } else {
        QWidget::mousePressEvent(event);
        event->ignore();
    }
}

void VideoPlayerMpv::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    event->ignore();
}

void VideoPlayerMpv::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    event->ignore();
}

void VideoPlayerMpv::setLoopPlayback(bool mode)
{
    m_mpv->setRepeat(mode);
}

void VideoPlayerMpv::show()
{
    QWidget::show();
}

void VideoPlayerMpv::hide()
{
    QWidget::hide();
}

VideoPlayer *CreatePlayerWidget()
{
    return new VideoPlayerMpv();
}
