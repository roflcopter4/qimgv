#pragma once

#include "gui/overlays/VideoControls.h"

struct VideoControlsStateBuffer {
    int duration = 0;
    int position = 0;
    bool paused = true;
    bool videoMuted = true;
    PlaybackMode mode;
};

class VideoControlsProxyWrapper : public QObject {
    Q_OBJECT
public:
    explicit VideoControlsProxyWrapper(FloatingWidgetContainer *parent = nullptr);
    ~VideoControlsProxyWrapper() override;
    void init();

    void show();
    void hide();

    ND bool underMouse() const;
    ND bool isVisible() const;

signals:
    void seek(int pos);
    void seekForward();
    void seekBackward();

public slots:
    void setPlaybackDuration(int);
    void setPlaybackPosition(int);
    void setMode(PlaybackMode);
    void onPlaybackPaused(bool);
    void onVideoMuted(bool);

private:
    FloatingWidgetContainer *container;
    VideoControls *videoControls;
    VideoControlsStateBuffer stateBuf;
};