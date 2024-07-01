#include "../../qimgv/Common.h"
#include "mpvwidget.h"
#include <stdexcept>

namespace qimgv {

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod(static_cast<MpvWidget *>(ctx), "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *, char const *name)
{
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

MpvWidget::MpvWidget(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NativeWindow);
    setMouseTracking(true);

    mpv = mpv_create();
    if (!mpv)
        throw std::runtime_error("could not create mpv context");

#ifdef _DEBUG
    mpv_set_option_string(mpv, "terminal", "yes");
    mpv_set_option_string(mpv, "msg-level", "all=v");
    //mpv_set_option_string(mpv, "msg-level", "all=debug");
#endif

    // Critically, we must tell MPV to use our application's main window.
    WId windowId = winId();
    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &windowId);

    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Request hw decoding, just for testing.
    mpv_set_option_string(mpv, "hwdec", "auto");
    // Loop video
    setRepeat(true);
    // Unmute
    setMuted(false);

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_set_wakeup_callback(mpv, wakeup, this);
}

MpvWidget::~MpvWidget()
{
    mpv_terminate_destroy(mpv);
}

void MpvWidget::command(QVariant const &params) const
{
    mpv::qt::command(mpv, params);
}

void MpvWidget::setProperty(QString const &name, QVariant const &value) const
{
    mpv::qt::set_property(mpv, name, value);
}

QVariant MpvWidget::getProperty(QString const &name) const
{
    return mpv::qt::get_property(mpv, name);
}

void MpvWidget::setOption(QString const &name, QVariant const &value) const
{
    mpv::qt::set_property(mpv, name, value);
}

void MpvWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;
        handle_mpv_event(event);
    }
}

void MpvWidget::handle_mpv_event(mpv_event const *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        auto *prop = static_cast<mpv_event_property *>(event->data);
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *static_cast<double *>(prop->data);
                Q_EMIT positionChanged(static_cast<int>(time));
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *static_cast<double *>(prop->data);
                Q_EMIT durationChanged(static_cast<int>(time));
            } else if (prop->format == MPV_FORMAT_NONE) {
                Q_EMIT playbackFinished();
            }
        } else if (strcmp(prop->name, "pause") == 0) {
            int mode = *static_cast<int *>(prop->data);
            Q_EMIT videoPaused(mode == 1);
        }
        break;
    }
    default:
        break;
    }
}

void MpvWidget::setMuted(bool mode) const
{
    mpv::qt::set_property(mpv, u"mute"_s, mode ? u"yes"_s : u"no"_s);
}

bool MpvWidget::muted() const
{
    return mpv::qt::get_property(mpv, u"mute"_s).toBool();
}

int MpvWidget::volume() const
{
    return mpv::qt::get_property(mpv, u"volume"_s).toInt();
}

void MpvWidget::setVolume(int vol) const
{
    mpv::qt::set_property(mpv, u"volume"_s, qBound(0, vol, 100));
}

void MpvWidget::setRepeat(bool mode) const
{
    mpv::qt::set_property(mpv, u"loop-file"_s, mode ? u"inf"_s : u"no"_s);
}

} // namespace qimgv
