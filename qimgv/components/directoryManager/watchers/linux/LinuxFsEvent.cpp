#include <QDebug>

#include "LinuxFsEvent.h"

LinuxFsEvent::LinuxFsEvent(std::unique_ptr<char[]> data, uint dataSize)
    : mData(std::move(data)),
      mDataSize(dataSize)
{

}

LinuxFsEvent::~LinuxFsEvent()
{
}

uint LinuxFsEvent::dataSize() const
{
    return mDataSize;
}

void LinuxFsEvent::setDataSize(uint dataSize)
{
    mDataSize = dataSize;
}

char *LinuxFsEvent::data() const
{
    return mData.get();
}

void LinuxFsEvent::setData(char *newData)
{
    mData = std::unique_ptr<char[]>(newData);
}

void LinuxFsEvent::setData(std::unique_ptr<char[]> newData)
{
    mData = std::move(newData);
}
