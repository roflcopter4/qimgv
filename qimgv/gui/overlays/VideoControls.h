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

  public Q_SLOTS:
    void setPlaybackDuration(int);
    void setPlaybackPosition(int);
    void onPlaybackPaused(bool);
    void onVideoMuted(bool);
    void setMode(PlaybackMode _mode);

  Q_SIGNALS:
    void seek(int pos);
    void seekForward();
    void seekBackward();

  private Q_SLOTS:
    void readSettings();

  private:
    Ui::VideoControls *ui;
    int                lastPosition;
    PlaybackMode       mode;
};
