#include "ChangeLogWindow.h"
#include "ui_ChangeLogWindow.h"

ChangelogWindow::ChangelogWindow(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      ui(new Ui::ChangelogWindow)
{
    ui->setupUi(this);
    hide();
    setPosition(FloatingWidget::Position::Center);
    connect(ui->closeButton, &QPushButton::pressed, this, &ChangelogWindow::hide);
    connect(ui->shutUpButton, &QPushButton::pressed, this, &ChangelogWindow::hideAndShutUp);

    if (parent)
        setContainerSize(parent->size());
}

void ChangelogWindow::hideAndShutUp()
{
    settings->setShowChangelogs(false);
    hide();
}

ChangelogWindow::~ChangelogWindow()
{
    delete ui;
}

void ChangelogWindow::setText(QString const &text)
{
    ui->textBrowser->setText(text);
}

void ChangelogWindow::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

// move to base class?
void ChangelogWindow::wheelEvent(QWheelEvent *event)
{
    event->accept();
}

void ChangelogWindow::keyPressEvent(QKeyEvent *event)
{
    QString key = ActionManager::keyForNativeScancode(event->nativeScanCode());
    if (key == u"Esc"_sv) {
        event->accept();
        hide();
    }
}

void ChangelogWindow::show()
{
    OverlayWidget::show();
    ui->closeButton->setFocus();
}

void ChangelogWindow::hide()
{
    ui->closeButton->clearFocus();
    ui->shutUpButton->clearFocus();
    OverlayWidget::hide();
}
