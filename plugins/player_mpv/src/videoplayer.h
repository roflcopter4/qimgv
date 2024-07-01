#pragma once

#include "../../qimgv/Common.h"
#include <QWidget>

namespace qimgv {

class VideoPlayer : public QWidget
{
    Q_OBJECT

  public:
    explicit VideoPlayer(QWidget *parent = nullptr);

    virtual bool showVideo(QString const &file) = 0;
    virtual void seek(int64_t pos)              = 0;
    virtual void seekRelative(int64_t pos)      = 0;
    virtual void pauseResume()                  = 0;
    virtual void frameStep()                    = 0;
    virtual void frameStepBack()                = 0;
    virtual void stop()                         = 0;
    virtual void setPaused(bool mode)           = 0;
    virtual void setMuted(bool)                 = 0;
    virtual void volumeUp()                     = 0;
    virtual void volumeDown()                   = 0;
    virtual void setVolume(int)                 = 0;
    virtual void setVideoUnscaled(bool mode)    = 0;
    virtual void setLoopPlayback(bool mode)     = 0;

    ND virtual bool muted() const  = 0;
    ND virtual int  volume() const = 0;

  Q_SIGNALS:
    void durationChanged(int value);
    void positionChanged(int value);
    void videoPaused(bool);
    void playbackFinished();

  public Q_SLOTS:
    virtual void show();
    virtual void hide();
};

} // namespace qimgv
