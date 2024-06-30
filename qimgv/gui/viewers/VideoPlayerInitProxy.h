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

class VideoPlayerInitProxy : public VideoPlayer
{
    Q_OBJECT

  public:
    explicit VideoPlayerInitProxy(QWidget *parent = nullptr);
    ~VideoPlayerInitProxy() override;
    DELETE_COPY_MOVE_ROUTINES(VideoPlayerInitProxy);

    bool showVideo(QString const &file) override;
    void seek(int pos) override;
    void seekRelative(int pos) override;
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
    ND int volume() const override;

    ND bool isInitialized() const;
    ND auto getPlayer() -> QSharedPointer<VideoPlayer>;

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    bool initPlayer();

  public Q_SLOTS:
    void show() override;
    void hide() override;

  private Q_SLOTS:
    void onSettingsChanged();

  Q_SIGNALS:
    void playbackFinished();

  private:
    QSharedPointer<VideoPlayer> player;

    QVBoxLayout *layout;
    QLabel      *errorLabel;
    QString     libFile;
    QStringList libDirs;
    QLibrary    playerLib;
};
