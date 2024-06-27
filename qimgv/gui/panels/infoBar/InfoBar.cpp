#include "InfoBar.h"
#include "ui_InfoBar.h"


InfoBar::InfoBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoBar)
{
    ui->setupUi(this);
    ui->path->setText(QS("No file opened."));
}

InfoBar::~InfoBar() {
    delete ui;
}

void InfoBar::setInfo(QString const &position, QString const &fileName, QString const &info) {
    ui->index->setText(position);
    ui->path->setText(fileName);
    ui->info->setText(info);
}

void InfoBar::wheelEvent(QWheelEvent *event) {
    event->accept();
}

void InfoBar::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}