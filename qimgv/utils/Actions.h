#pragma once

#include <QMap>
#include <QString>
#include <QVersionNumber>

class Actions
{
  public:
    static Actions *getInstance();

    ND auto getMap() const -> QMap<QString, QVersionNumber> const &;
    ND auto getList() const -> QList<QString>;

  private:
    Actions();
    void init();

    QMap<QString, QVersionNumber> mActions;
};

extern Actions *appActions;
