#pragma once

#include "../../qimgv/Common.h"
#include "qthelper.hpp"
#include <mpv/client.h>
#include <mpv/render_gl.h>

#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLWidget>
#include <QTimer>
#include <QtCore/QMetaObject>

namespace qimgv {

class MpvWidget final : public QWidget
{
    Q_OBJECT

  public:
    explicit MpvWidget(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::Widget);
    ~MpvWidget() override;
    DELETE_COPY_MOVE_ROUTINES(MpvWidget);

    void command(QVariant const &params) const;
    void setOption(QString const &name, QVariant const &value) const;
    void setProperty(QString const &name, QVariant const &value) const;
    void setMuted(bool mode) const;
    void setRepeat(bool mode) const;
    void setVolume(int vol) const;

    ND QVariant getProperty(QString const &name) const;
    ND bool     muted() const;
    ND int      volume() const;

  private:
    void handle_mpv_event(mpv_event const *event);

  Q_SIGNALS:
    void durationChanged(int value);
    void positionChanged(int value);
    void videoPaused(bool);
    void playbackFinished();

  private Q_SLOTS:
    void on_mpv_events();

  private:
    mpv_handle         *mpv;
    mpv_render_context *mpv_gl = nullptr;
};

} // namespace qimgv
