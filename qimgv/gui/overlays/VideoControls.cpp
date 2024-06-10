#include "VideoControls.h"
#include "ui_VideoControls.h"

VideoControls::VideoControls(FloatingWidgetContainer *parent) :
    OverlayWidget(parent),
    ui(new Ui::VideoControls)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_NoMousePropagation, true);
    hide();
    ui->pauseButton->setIconPath(QS(":res/icons/common/buttons/videocontrols/play24.png"));
    ui->pauseButton->setAction(QS("pauseVideo"));
    ui->prevFrameButton->setIconPath(QS(":res/icons/common/buttons/videocontrols/skip-backwards24.png"));
    ui->prevFrameButton->setAction(QS("frameStepBack"));
    ui->nextFrameButton->setIconPath(QS(":res/icons/common/buttons/videocontrols/skip-forward24.png"));
    ui->nextFrameButton->setAction(QS("frameStep"));
    ui->muteButton->setIconPath(QS(":/res/icons/common/buttons/videocontrols/mute-on24.png"));
    ui->muteButton->setAction(QS("toggleMute"));

    lastPosition = -1;

    readSettings();
    connect(settings, &Settings::settingsChanged, this, &VideoControls::readSettings);

    connect(ui->seekBar, &VideoSlider::sliderMovedX, this, &VideoControls::seek);

    if(parent)
        setContainerSize(parent->size());
}

void VideoControls::readSettings() {
    if(settings->panelEnabled() && settings->panelPosition() == PanelPosition::BOTTOM)
        setPosition(FloatingWidgetPosition::TOP);
    else
        setPosition(FloatingWidgetPosition::BOTTOM);
}

VideoControls::~VideoControls() {
    delete ui;
}

void VideoControls::setMode(PlaybackMode _mode) {
    mode = _mode;
    ui->muteButton->setVisible( (mode == PlaybackMode::VIDEO) );
}

void VideoControls::setPlaybackDuration(int duration) {
    QString durationStr;
    if(mode == PlaybackMode::VIDEO) {
        int _time = duration;
        int hours = _time / 3600;
        _time -= hours * 3600;
        int minutes = _time / 60;
        int seconds = _time - minutes * 60;
        durationStr = QS("%1").arg(minutes, 2, 10, QChar(u'0')) + u':' +
                      QS("%1").arg(seconds, 2, 10, QChar(u'0'));
        if(hours)
            durationStr.prepend(QS("%1").arg(hours, 2, 10, QChar(u'0')) + u':');
    } else {
        durationStr = QString::number(duration);
    }
    ui->seekBar->setRange(0, duration - 1);
    ui->durationLabel->setText(durationStr);
    ui->positionLabel->setText(durationStr);
    recalculateGeometry();
    ui->positionLabel->setText(QS(""));
}

void VideoControls::setPlaybackPosition(int Position) {
    if(Position == lastPosition)
        return;
    QString positionStr;
    if(mode == PlaybackMode::VIDEO) {
        int _time = Position;
        int hours = _time / 3600;
        _time -= hours * 3600;
        int minutes = _time / 60;
        int seconds = _time - minutes * 60;
        positionStr = QS("%1").arg(minutes, 2, 10, QChar(u'0')) + u':' +
                      QS("%1").arg(seconds, 2, 10, QChar(u'0'));
        if(hours)
            positionStr.prepend(QS("%1").arg(hours, 2, 10, QChar(u'0')) + u':');
    } else {
        positionStr = QString::number(Position + 1);
    }
    ui->positionLabel->setText(positionStr);
    ui->seekBar->blockSignals(true);
    ui->seekBar->setValue(Position);
    ui->seekBar->blockSignals(false);
    lastPosition = Position;
}

void VideoControls::onPlaybackPaused(bool newMode) {
    if(newMode)
        ui->pauseButton->setIconPath(QS(":res/icons/common/buttons/videocontrols/play24.png"));
    else
        ui->pauseButton->setIconPath(QS(":res/icons/common/buttons/videocontrols/pause24.png"));
}

void VideoControls::onVideoMuted(bool newMode) {
    if(newMode)
        ui->muteButton->setIconPath(QS(":res/icons/common/buttons/videocontrols/mute-on24.png"));
    else
        ui->muteButton->setIconPath(QS(":res/icons/common/buttons/videocontrols/mute-off24.png"));
}
