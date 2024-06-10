#include "InfoBarProxy.h"

InfoBarProxy::InfoBarProxy(QWidget *parent)
    : QWidget(parent),
      infoBar(nullptr),
      layout(new QVBoxLayout(this))
{
    setAccessibleName(QS("InfoBarProxy"));
    setMinimumHeight(23);
    setMaximumHeight(23);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

InfoBarProxy::~InfoBarProxy()
{
    if (infoBar)
        infoBar->deleteLater();
}

void InfoBarProxy::setInfo(QString const &position, QString const &fileName, QString const &info)
{
    if (infoBar) {
        infoBar->setInfo(position, fileName, info);
    } else {
        stateBuf.position = position;
        stateBuf.fileName = fileName;
        stateBuf.info     = info;
    }
}

void InfoBarProxy::init()
{
    if (infoBar)
        return;
    infoBar = new InfoBar(this);
    setFocusProxy(infoBar);
    layout->addWidget(infoBar);
    setLayout(layout);
    if (!stateBuf.fileName.isEmpty())
        infoBar->setInfo(stateBuf.position, stateBuf.fileName, stateBuf.info);
}

void InfoBarProxy::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
