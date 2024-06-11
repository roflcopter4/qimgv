#include <QDebug>

#include "LinuxFsEvent.h"

LinuxFsEvent::LinuxFsEvent(char *data, uint dataSize)
    : mData(data),
      mDataSize(dataSize)
{}

LinuxFsEvent::~LinuxFsEvent()
{
    delete[] mData;
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
    return mData;
}

void LinuxFsEvent::setData(char *data)
{
    mData = data;
}
