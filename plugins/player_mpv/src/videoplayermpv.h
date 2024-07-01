#pragma once

#include "../../qimgv/Common.h"
#include "videoplayer.h"

#include <QKeyEvent>

#if defined QIMGV_PLAYER_MPV_LIBRARY
 #define TEST_COMMON_DLLSPEC Q_DECL_EXPORT
#else
 #define TEST_COMMON_DLLSPEC Q_DECL_IMPORT
#endif

namespace qimgv {

class MpvWidget;

class VideoPlayerMpv final : public VideoPlayer
{
    Q_OBJECT

  public:
    explicit VideoPlayerMpv(QWidget *parent = nullptr);

    bool showVideo(QString const &file) override;
    void setVideoUnscaled(bool mode) override;

    ND int volume() const override;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  public Q_SLOTS:
    void seek(int64_t pos) override;
    void seekRelative(int64_t pos) override;
    void pauseResume() override;
    void frameStep() override;
    void frameStepBack() override;
    void stop() override;
    void setPaused(bool mode) override;
    void setMuted(bool) override;
    void volumeUp() override;
    void volumeDown() override;
    void setVolume(int) override;
    void show() override;
    void hide() override;
    void setLoopPlayback(bool mode) override;

    ND bool muted() const override;

  Q_SIGNALS:
    void playbackFinished();

  private Q_SLOTS:
    void readSettings();

  private:
    MpvWidget *m_mpv;
};

} // namespace qimgv

extern "C" ND TEST_COMMON_DLLSPEC qimgv::VideoPlayer *CreatePlayerWidget(QWidget *parent);
