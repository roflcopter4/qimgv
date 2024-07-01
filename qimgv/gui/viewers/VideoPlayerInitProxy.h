// performs lazy initialization

#pragma once

#include "Settings.h"
#include "VideoPlayer.h"
#include <QDebug>
#include <QFileInfo>
#include <QLabel>
#include <QLibrary>
#include <QPainter>
#include <QVBoxLayout>
#include <memory>
#ifdef Q_OS_WINDOWS
# include "Platform.h"
#endif

class VideoPlayerInitProxy : public VideoPlayer
{
    Q_OBJECT

  public:
    explicit VideoPlayerInitProxy(QWidget *parent = nullptr);
    ~VideoPlayerInitProxy() override;
    DELETE_COPY_MOVE_ROUTINES(VideoPlayerInitProxy);

    bool showVideo(QString const &file) override;
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
    void setVideoUnscaled(bool mode) override;
    void setLoopPlayback(bool mode) override;

    ND bool muted() const override;
    ND int  volume() const override;
    ND bool isInitialized() const;
    ND auto getPlayer() -> QSharedPointer<VideoPlayer>;

  private:
    bool initPlayer();

  Q_SIGNALS:
    void playbackFinished();

  public Q_SLOTS:
    void show() override;
    void hide() override;

  private Q_SLOTS:
    void onSettingsChanged();

  private:
    QSharedPointer<VideoPlayer> player;

    QVBoxLayout *layout;
    QLabel      *errorLabel;
    QString      libFile;
    QStringList  libDirs;
    QLibrary     playerLib;

#ifdef Q_OS_WINDOWS
    HMODULE hPlayerModule = nullptr;
#endif
};
