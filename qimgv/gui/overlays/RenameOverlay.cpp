#include "RenameOverlay.h"
#include "ui_RenameOverlay.h"

RenameOverlay::RenameOverlay(FloatingWidgetContainer *parent)
    : OverlayWidget(parent),
      ui(new Ui::RenameOverlay)
{
    ui->setupUi(this);
    connect(ui->cancelButton, &QPushButton::clicked, this, &RenameOverlay::onCancel);
    connect(ui->closeButton, &IconButton::clicked, this, &RenameOverlay::hide);
    connect(ui->okButton, &QPushButton::clicked, this, &RenameOverlay::rename);
    ui->okButton->setHighlighted(true);
    ui->closeButton->setIconPath(u":res/icons/common/overlay/close-dim16.png"_s);
    ui->headerIcon->setIconPath(u":res/icons/common/overlay/edit16.png"_s);
    setPosition(FloatingWidget::Position::Center);
    setAcceptKeyboardFocus(true);

    keyFilter.append(actionManager->shortcutsForAction(u"exit"_s));
    keyFilter.append(actionManager->shortcutsForAction(u"renameFile"_s));

    hide();
    if (parent)
        setContainerSize(parent->size());
}

RenameOverlay::~RenameOverlay()
{
    delete ui;
}

void RenameOverlay::show()
{
    selectName();
    OverlayWidget::show();
    QTimer::singleShot(0, ui->fileName, qOverload<>(&QWidget::setFocus));
}

void RenameOverlay::hide()
{
    OverlayWidget::hide();
}

void RenameOverlay::setName(QString const &name)
{
    ui->fileName->setText(name);
    origName = name;
    selectName();
}

void RenameOverlay::setBackdropEnabled(bool mode)
{
    if (backdrop == mode)
        return;
    backdrop = mode;
    recalculateGeometry();
}

void RenameOverlay::recalculateGeometry()
{
    if (!backdrop)
        OverlayWidget::recalculateGeometry();
    else // expand
        setGeometry(0, 0, containerSize().width(), containerSize().height());
}

void RenameOverlay::selectName()
{
    auto end = ui->fileName->text().lastIndexOf(u'.');
    if (end < 0)
        end = ui->fileName->text().size();
    ui->fileName->setSelection(0, end);
}

void RenameOverlay::rename()
{
    if (ui->fileName->text().isEmpty())
        return;
    hide();
    emit renameRequested(ui->fileName->text());
}

void RenameOverlay::onCancel()
{
    hide();
    ui->fileName->setText(origName);
}

void RenameOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        event->accept();
        onCancel();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        event->accept();
        rename();
    } else {
        auto shortcut = ShortcutBuilder::fromEvent(event);
        if (!shortcut.isEmpty() && keyFilter.contains(shortcut))
            event->ignore();
        else
            event->accept();
    }
}

void RenameOverlay::mousePressEvent(QMouseEvent *event)
{
    event->accept();
    if (qApp->widgetAt(mapToGlobal(event->pos())) == this)
        onCancel();
}
