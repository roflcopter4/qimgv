#pragma once

#include "gui/panels/infoBar/InfoBar.h"
#include <QVBoxLayout>

struct InfoBarStateBuffer {
    QString position;
    QString fileName;
    QString info;
};

class InfoBarProxy : public QWidget {
    Q_OBJECT
public:
    explicit InfoBarProxy(QWidget *parent = nullptr);
    ~InfoBarProxy() override;

    void init();
public slots:
    void setInfo(QString const &position, QString const &fileName, QString const &info);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    InfoBarStateBuffer stateBuf;
    InfoBar *infoBar;
    QVBoxLayout *layout;
};
