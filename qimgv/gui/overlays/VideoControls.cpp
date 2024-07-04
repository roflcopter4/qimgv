#include "VideoControls.h"
#include "ui_VideoControls.h"

VideoControls::VideoControls(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      ui(new Ui::VideoControls),
      lastPosition(-1)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_NoMousePropagation);
    hide();
    ui->pauseButton->setIconPath(u":res/icons/common/buttons/videocontrols/play24.png"_s);
    ui->pauseButton->setAction(u"pauseVideo"_s);
    ui->prevFrameButton->setIconPath(u":res/icons/common/buttons/videocontrols/skip-backwards24.png"_s);
    ui->prevFrameButton->setAction(u"frameStepBack"_s);
    ui->nextFrameButton->setIconPath(u":res/icons/common/buttons/videocontrols/skip-forward24.png"_s);
    ui->nextFrameButton->setAction(u"frameStep"_s);
    ui->muteButton->setIconPath(u":/res/icons/common/buttons/videocontrols/mute-on24.png"_s);
    ui->muteButton->setAction(u"toggleMute"_s);

    lastPosition = -1;

    readSettings();
    connect(settings, &Settings::settingsChanged, this, &VideoControls::readSettings);

    connect(ui->seekBar, &VideoSlider::sliderMovedX, this, &VideoControls::seek);

    if (parent)
        setContainerSize(parent->size());
}

void VideoControls::readSettings()
{
    if (settings->panelEnabled() && settings->panelPosition() == PanelPosition::BOTTOM)
        setPosition(FloatingWidget::Position::Top);
    else
        setPosition(FloatingWidget::Position::Bottom);
}

VideoControls::~VideoControls()
{
    delete ui;
}

void VideoControls::setMode(PlaybackMode _mode)
{
    mode = _mode;
    ui->muteButton->setVisible((mode == PlaybackMode::VIDEO));
}

void VideoControls::setPlaybackDuration(int64_t duration)
{
    QString durationStr;
    if (mode == PlaybackMode::VIDEO) {
        int64_t time  = duration;
        int64_t hours = time / 3600;
        time -= hours * 3600;
        int64_t minutes = time / 60;
        int64_t seconds = time - minutes * 60;

        durationStr = u"%1:%2"_s.arg(minutes, 2, 10, QChar(u'0')).arg(seconds, 2, 10, QChar(u'0'));
        if (hours)
            durationStr.prepend(u"%1:"_s.arg(hours, 2, 10, QChar(u'0')));
    } else {
        durationStr = QString::number(duration);
    }
    ui->seekBar->setRange(0, static_cast<int>(duration - 1));
    ui->durationLabel->setText(durationStr);
    ui->positionLabel->setText(durationStr);
    recalculateGeometry();
    ui->positionLabel->setText(u""_s);
}

void VideoControls::setPlaybackPosition(int64_t Position)
{
    if (Position == lastPosition)
        return;
    QString positionStr;
    if (mode == PlaybackMode::VIDEO) {
        int64_t time  = Position;
        int64_t hours = time / 3600;
        time -= hours * 3600;
        int64_t minutes = time / 60;
        int64_t seconds = time - minutes * 60;

        positionStr = u"%1:%2"_s.arg(minutes, 2, 10, QChar(u'0')).arg(seconds, 2, 10, QChar(u'0'));
        if (hours)
            positionStr.prepend(u"%1:"_s.arg(hours, 2, 10, QChar(u'0')));
    } else {
        positionStr = QString::number(Position + 1);
    }
    ui->positionLabel->setText(positionStr);
    ui->seekBar->blockSignals(true);
    ui->seekBar->setValue(static_cast<int>(Position));
    ui->seekBar->blockSignals(false);
    lastPosition = static_cast<int>(Position);
}

void VideoControls::onPlaybackPaused(bool newMode)
{
    if (newMode)
        ui->pauseButton->setIconPath(u":res/icons/common/buttons/videocontrols/play24.png"_s);
    else
        ui->pauseButton->setIconPath(u":res/icons/common/buttons/videocontrols/pause24.png"_s);
}

void VideoControls::onVideoMuted(bool newMode)
{
    if (newMode)
        ui->muteButton->setIconPath(u":res/icons/common/buttons/videocontrols/mute-on24.png"_s);
    else
        ui->muteButton->setIconPath(u":res/icons/common/buttons/videocontrols/mute-off24.png"_s);
}
