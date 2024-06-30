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
    WatcherEvent(QString name, uint32_t cookie, int timerId, Type type = Type::None);
    ~WatcherEvent();

    DEFAULT_COPY_MOVE_CONSTRUCTORS(WatcherEvent);

    ND auto name() const -> QString;
       void setName(QString const &name);
    ND auto cookie() const -> uint32_t;
       void setCookie(uint32_t cookie);
    ND auto timerId() const -> int;
       void setTimerId(int timerId);
    ND auto type() const -> Type;
       void setType(Type type);

  private:
    QString  mName;
    uint32_t mCookie;
    int      mTimerId;
    Type     mType;
};
