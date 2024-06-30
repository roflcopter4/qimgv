#include <QDebug>

#include "LinuxFsEvent.h"

LinuxFsEvent::LinuxFsEvent(std::unique_ptr<char[]> data, uint32_t dataSize)
    : mData(std::move(data)),
      mDataSize(dataSize)
{}

LinuxFsEvent::~LinuxFsEvent() = default;

uint32_t LinuxFsEvent::dataSize() const
{
    return mDataSize;
}

void LinuxFsEvent::setDataSize(uint32_t dataSize)
{
    mDataSize = dataSize;
}

char *LinuxFsEvent::data() const
{
    return mData.get();
}

void LinuxFsEvent::setData(char *data)
{
    mData = std::unique_ptr<char[]>(data);
}

void LinuxFsEvent::setData(std::unique_ptr<char[]> data)
{
    mData = std::move(data);
}
