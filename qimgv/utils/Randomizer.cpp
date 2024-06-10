#include "Randomizer.h"

Randomizer::Randomizer()
{
    setCount(0);
}

Randomizer::Randomizer(qsizetype count)
    : currentIndex(0)
{
    setCount(count);
}

void Randomizer::setCount(qsizetype count)
{
    vec.resize(count);
    fill();
}

void Randomizer::shuffle()
{
    static std::random_device dev;
    static std::mt19937       rng(dev());

    std::ranges::shuffle(vec, rng);
}

void Randomizer::setCurrent(qsizetype current)
{
    currentIndex = indexOf(current);
}

// this assumes our array contains shuffled int values [0 ... count]
// which it does in our case
qsizetype Randomizer::indexOf(qsizetype item) const
{
    qsizetype index = -1;
    qsizetype i     = 0;

    if (item >= static_cast<qsizetype>(vec.size()))
        return index;
    for (uint32_t it : vec) {
        if (it == item) {
            index = i;
            break;
        }
        ++i;
    }
    return index;
}

void Randomizer::fill()
{
    uint32_t i = 0;
    for (uint32_t &it : vec)
        it = i++;
}

void Randomizer::print() const
{
    qDebug() << u"---vector---";
    for (uint32_t it : vec)
        qDebug() << it;
    qDebug() << u"----end----";
}

uint32_t Randomizer::next()
{
    // re-shuffle when needed
    // because vector gets rearranged this will break prev()
    while (currentIndex == static_cast<qsizetype>(vec.size()) - 1) {
        uint32_t currentItem = vec[currentIndex];
        shuffle();
        setCurrent(currentItem);
    }
    return vec[currentIndex++];
}

uint32_t Randomizer::prev()
{
    while (currentIndex == 0) {
        uint32_t currentItem = vec[currentIndex];
        shuffle();
        setCurrent(currentItem);
    }
    return vec[--currentIndex];
}
