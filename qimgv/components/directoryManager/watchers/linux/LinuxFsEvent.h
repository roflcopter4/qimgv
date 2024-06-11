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
    void setData(std::unique_ptr<char[]> newData);
    [[deprecated("Prefer providing an std::unique_ptr")]]
    void setData(char *newData);

  private:
    std::unique_ptr<char[]> mData;
    uint mDataSize;
};
