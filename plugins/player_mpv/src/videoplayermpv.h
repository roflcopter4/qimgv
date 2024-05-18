#pragma once

#include "videoplayer.h"
#include <QKeyEvent>

#if defined QIMGV_PLAYER_MPV_LIBRARY
 #define TEST_COMMON_DLLSPEC Q_DECL_EXPORT
#else
 #define TEST_COMMON_DLLSPEC Q_DECL_IMPORT
#endif

class MpvWidget;

class VideoPlayerMpv final : public VideoPlayer
{
    Q_OBJECT

  public:
    explicit VideoPlayerMpv(QWidget *parent = nullptr);

    bool showVideo(QString const &file) override;
    void setVideoUnscaled(bool mode) override;
    [[nodiscard]] int volume() const override;

  public Q_SLOTS:
    void seek(int pos) override;
    void seekRelative(int pos) override;
    void pauseResume() override;
    void frameStep() override;
    void frameStepBack() override;
    void stop() override;
    void setPaused(bool mode) override;
    void setMuted(bool) override;
    [[nodiscard]] bool muted() const override;
    void volumeUp() override;
    void volumeDown() override;
    void setVolume(int) override;
    void show() override;
    void hide() override;
    void setLoopPlayback(bool mode) override;

  protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  Q_SIGNALS:
    void playbackFinished();

  private Q_SLOTS:
    void readSettings();

  private:
    MpvWidget *m_mpv;
};

extern "C" TEST_COMMON_DLLSPEC VideoPlayer *CreatePlayerWidget();
