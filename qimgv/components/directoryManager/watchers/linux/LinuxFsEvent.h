#pragma once

#include <QObject>

class LinuxFsEvent : public QObject
{
  public:
    LinuxFsEvent(std::unique_ptr<char[]> data, uint32_t dataSize);
    ~LinuxFsEvent() override;

    ND uint32_t dataSize() const;
    ND char *data() const;

    void setDataSize(uint32_t dataSize);
    [[deprecated("Do not use")]]
    void setData(char *data);
    void setData(std::unique_ptr<char[]> data);

  private:
    std::unique_ptr<char[]> mData;
    uint32_t mDataSize;
};
