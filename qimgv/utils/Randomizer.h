#pragma once

#include <QDebug>
#include <QString>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

class Randomizer
{
  public:
    Randomizer();
    explicit Randomizer(qsizetype count);

    void setCount(qsizetype count);
    void setCurrent(qsizetype current);
    void shuffle();
    void print() const;
    uint32_t next();
    uint32_t prev();

  private:
    qsizetype currentIndex = 0;
    std::vector<uint32_t> vec;

    void fill();
    ND qsizetype indexOf(qsizetype item) const;
};
