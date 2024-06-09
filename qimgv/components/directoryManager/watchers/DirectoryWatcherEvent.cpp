#include "DirectoryWatcherEvent.h"
#include <QDebug>

WatcherEvent::WatcherEvent(QString const &name, int timerId, Type type)
    : WatcherEvent(name, 0, timerId, type)
{}

WatcherEvent::WatcherEvent(QString name, uint cookie, int timerId, Type type)
    : mName(std::move(name)),
      mCookie(cookie),
      mTimerId(timerId),
      mType(type)
{}

WatcherEvent::~WatcherEvent() = default;

QString WatcherEvent::name() const
{
    return mName;
}

void WatcherEvent::setName(QString const &name)
{
    mName = name;
}

WatcherEvent::Type WatcherEvent::type() const
{
    return mType;
}

void WatcherEvent::setType(Type type)
{
    mType = type;
}

int WatcherEvent::timerId() const
{
    return mTimerId;
}

void WatcherEvent::setTimerId(int timerId)
{
    mTimerId = timerId;
}

uint WatcherEvent::cookie() const
{
    return mCookie;
}

void WatcherEvent::setCookie(uint cookie)
{
    mCookie = cookie;
}
