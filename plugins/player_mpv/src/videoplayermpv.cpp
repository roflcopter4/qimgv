#include "mpvwidget.h"
#include "videoplayermpv.h"
#include <QFileDialog>
#include <QLayout>
#include <QPushButton>
#include <QSlider>

namespace qimgv {

// TODO: window flashes white when opening a video (straight from file manager)
VideoPlayerMpv::VideoPlayerMpv(QWidget *parent)
    : VideoPlayer(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    m_mpv = new MpvWidget(this);

    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->addWidget(m_mpv);
    setLayout(vl);

    setFocusPolicy(Qt::NoFocus);
    m_mpv->setFocusPolicy(Qt::NoFocus);

    readSettings();

    connect(m_mpv, &MpvWidget::durationChanged,  this, &VideoPlayerMpv::durationChanged);
    connect(m_mpv, &MpvWidget::positionChanged,  this, &VideoPlayerMpv::positionChanged);
    connect(m_mpv, &MpvWidget::videoPaused,      this, &VideoPlayerMpv::videoPaused);
    connect(m_mpv, &MpvWidget::playbackFinished, this, &VideoPlayerMpv::playbackFinished);
}

bool VideoPlayerMpv::showVideo(QString const &file)
{
    if (file.isEmpty())
        return false;
    m_mpv->command(QStringList() << u"loadfile"_s << file);
    setPaused(false);
    return true;
}

void VideoPlayerMpv::seek(int64_t pos)
{
    m_mpv->command(QVariantList() << u"seek"_s << pos << u"absolute"_s);
}

void VideoPlayerMpv::seekRelative(int64_t pos)
{
    m_mpv->command(QVariantList() << u"seek"_s << pos << u"relative"_s);
}

void VideoPlayerMpv::pauseResume()
{
    bool paused = m_mpv->getProperty(u"pause"_s).toBool();
    setPaused(!paused);
}

void VideoPlayerMpv::frameStep()
{
    m_mpv->command(QVariantList() << u"frame-step"_s);
}

void VideoPlayerMpv::frameStepBack()
{
    m_mpv->command(QVariantList() << u"frame-back-step"_s);
}

void VideoPlayerMpv::stop()
{
    m_mpv->command(QVariantList() << u"stop"_s);
}

void VideoPlayerMpv::setPaused(bool mode)
{
    m_mpv->setProperty(u"pause"_s, mode);
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
    m_mpv->setOption(u"video-unscaled"_s, mode ? u"downscale-big"_s : u"no"_s);
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
        pauseResume();
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

} // namespace qimgv

extern "C" qimgv::VideoPlayer *CreatePlayerWidget(QWidget *parent)
{
    return new qimgv::VideoPlayerMpv(parent);
}
