// performs lazy initialization

#pragma once

#include "settings.h"
#include "videoplayer.h"
#include <QDebug>
#include <QFileInfo>
#include <QLabel>
#include <QLibrary>
#include <QPainter>
#include <QVBoxLayout>
#include <memory>

class VideoPlayerInitProxy : public VideoPlayer
{
  public:
    explicit VideoPlayerInitProxy(QWidget *parent = nullptr);
    ~VideoPlayerInitProxy() override;

    bool showVideo(QString const &file) override;
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

    [[nodiscard]] int volume() const override;

    void setVideoUnscaled(bool mode) override;
    void setLoopPlayback(bool mode) override;
    bool isInitialized() const;

    std::shared_ptr<VideoPlayer> getPlayer();

  public Q_SLOTS:
    void show() override;
    void hide() override;

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    std::shared_ptr<VideoPlayer> player;
    QString     libFile;
    QStringList libDirs;
    QLibrary    playerLib;
    QVBoxLayout layout;
    QLabel     *errorLabel = nullptr;

    bool initPlayer();

  private Q_SLOTS:
    void onSettingsChanged();

  Q_SIGNALS:
    void playbackFinished();
};
