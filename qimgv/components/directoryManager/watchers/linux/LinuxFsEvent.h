#pragma once

#include <QObject>

class LinuxFsEvent : public QObject
{
  public:
    LinuxFsEvent(std::unique_ptr<char[]> data, uint dataSize);
    ~LinuxFsEvent() override;

    ND uint dataSize() const;
    ND char *data() const;

    void setDataSize(uint dataSize);
    [[deprecated("Do not use")]]
    void setData(char *newData);
    void setData(std::unique_ptr<char[]> newData);

  private:
    std::unique_ptr<char[]> mData;
    uint mDataSize;
};
