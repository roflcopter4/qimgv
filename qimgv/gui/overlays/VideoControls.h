#pragma once

#include "gui/customWidgets/OverlayWidget.h"
#include "Settings.h"
#include <QPushButton>

namespace Ui {
class VideoControls;
}

enum class PlaybackMode : uint8_t {
    ANIMATION,
    VIDEO,
};

class VideoControls : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit VideoControls(FloatingWidgetContainer *parent = nullptr);
    ~VideoControls() override;
    DELETE_COPY_MOVE_ROUTINES(VideoControls);

  Q_SIGNALS:
    void seek(int64_t pos);
    void seekForward();
    void seekBackward();

  public Q_SLOTS:
    void setPlaybackDuration(int64_t);
    void setPlaybackPosition(int64_t);
    void onPlaybackPaused(bool);
    void onVideoMuted(bool);
    void setMode(PlaybackMode);

  private Q_SLOTS:
    void readSettings();

  private:
    Ui::VideoControls *ui;
    int                lastPosition;
    PlaybackMode       mode;
};
