#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include <QDebug>
#include <QString>

#include "Common.h"

class Randomizer
{
  public:
    Randomizer();
    explicit Randomizer(qsizetype count);

    void     setCount(qsizetype count);
    uint32_t next();
    uint32_t prev();

    void shuffle();
    void print() const;
    void setCurrent(qsizetype current);

  private:
    qsizetype currentIndex;
    std::vector<uint32_t> vec;

    void         fill();
    ND qsizetype indexOf(qsizetype item) const;
};
