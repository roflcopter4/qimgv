#pragma once

#include <QString>

class WatcherEvent
{
  public:
    enum class Type {
        None,
        MovedFrom,
        MovedTo,
        Modify,
    };

    WatcherEvent(QString const &name, int timerId, Type type = Type::None);
    WatcherEvent(QString name, uint cookie, int timerId, Type type = Type::None);
    ~WatcherEvent();

    ND QString name() const;
       void    setName(QString const &name);
    ND uint    cookie() const;
       void    setCookie(uint cookie);
    ND int     timerId() const;
       void    setTimerId(int timerId);
    ND Type    type() const;
       void    setType(Type type);

  private:
    QString mName;
    uint    mCookie;
    int     mTimerId;
    Type    mType;
};
