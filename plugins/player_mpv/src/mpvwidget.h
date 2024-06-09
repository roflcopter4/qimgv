#pragma once

#include <QtCore/QMetaObject>
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>

#include <mpv/client.h>
#include <mpv/render_gl.h>
#include "qthelper.hpp"

#include <QDebug>
#include <ctime>
#include <QSurfaceFormat>
#include <QTimer>

class MpvWidget final : public QOpenGLWidget
{
    Q_OBJECT

  public:
    explicit MpvWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Widget);
    ~MpvWidget() override;

    void command(QVariant const &params) const;
    void setOption(QString const &name, QVariant const &value) const;
    void setProperty(QString const &name, QVariant const &value) const;
    void setMuted(bool mode) const;
    void setRepeat(bool mode) const;
    void setVolume(int vol) const;

    [[nodiscard]] QVariant getProperty(QString const &name) const;
    [[nodiscard]] bool     muted() const;
    [[nodiscard]] int      volume() const;

    // Related to this:
    // https://github.com/gnome-mpv/gnome-mpv/issues/245
    // Let's hope this won't break more than it fixes.
    [[nodiscard]] int width() const
    {
        return static_cast<int>(QOpenGLWidget::width() * devicePixelRatioF());
    }

    [[nodiscard]] int height() const
    {
        return static_cast<int>(QOpenGLWidget::height() * devicePixelRatioF());
    }

  Q_SIGNALS:
    void durationChanged(int value);
    void positionChanged(int value);
    void videoPaused(bool);
    void playbackFinished();

  protected:
    void initializeGL() override;
    void paintGL() override;

  private Q_SLOTS:
    void on_mpv_events();
    void maybeUpdate();

  private:
    void        handle_mpv_event(mpv_event const *event);
    static void on_update(void *ctx);

    mpv_handle         *mpv;
    mpv_render_context *mpv_gl;
};
