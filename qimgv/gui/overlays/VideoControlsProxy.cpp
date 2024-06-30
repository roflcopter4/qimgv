#include "VideoControlsProxy.h"

VideoControlsProxyWrapper::VideoControlsProxyWrapper(FloatingWidgetContainer *parent)
    : container(parent),
      videoControls(nullptr)
{}

VideoControlsProxyWrapper::~VideoControlsProxyWrapper()
{
    if (videoControls)
        videoControls->deleteLater();
}

void VideoControlsProxyWrapper::init()
{
    if (videoControls)
        return;
    videoControls = new VideoControls(container);

    connect(videoControls, &VideoControls::seekBackward, this, &VideoControlsProxyWrapper::seekBackward);
    connect(videoControls, &VideoControls::seekForward,  this, &VideoControlsProxyWrapper::seekForward);
    connect(videoControls, &VideoControls::seek,         this, &VideoControlsProxyWrapper::seek);

    videoControls->setMode(stateBuf.mode);
    videoControls->setPlaybackDuration(stateBuf.duration);
    videoControls->setPlaybackPosition(stateBuf.position);
    videoControls->onPlaybackPaused(stateBuf.paused);
    videoControls->onVideoMuted(stateBuf.videoMuted);
}

void VideoControlsProxyWrapper::show()
{
    init();
    videoControls->show();
}

void VideoControlsProxyWrapper::hide()
{
    if (videoControls)
        videoControls->hide();
}

void VideoControlsProxyWrapper::setPlaybackDuration(int duration)
{
    if (videoControls)
        videoControls->setPlaybackDuration(duration);
    else
        stateBuf.duration = duration;
}

void VideoControlsProxyWrapper::setPlaybackPosition(int position)
{
    if (videoControls)
        videoControls->setPlaybackPosition(position);
    else
        stateBuf.position = position;
}

void VideoControlsProxyWrapper::setMode(PlaybackMode mode)
{
    if (videoControls)
        videoControls->setMode(mode);
    else
        stateBuf.mode = mode;
}

void VideoControlsProxyWrapper::onPlaybackPaused(bool mode)
{
    if (videoControls)
        videoControls->onPlaybackPaused(mode);
    else
        stateBuf.paused = mode;
}

void VideoControlsProxyWrapper::onVideoMuted(bool mode)
{
    if (videoControls)
        videoControls->onVideoMuted(mode);
    else
        stateBuf.videoMuted = mode;
}

bool VideoControlsProxyWrapper::underMouse() const
{
    return videoControls ? videoControls->underMouse() : false;
}

bool VideoControlsProxyWrapper::isVisible() const
{
    return videoControls ? videoControls->isVisible() : false;
}
