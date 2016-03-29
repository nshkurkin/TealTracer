///
///
///
///
///
///

#include "Random.h"

using namespace util;

Random::Random(unsigned int seed) : generator(seed),
 distribution() {//std::numeric_limits<int>::min(),
// std::numeric_limits<int>::max()) {
    this->seed = seed;
}

unsigned int Random::getSeed() {return seed;}

int Random::nextInt() {return distribution(generator);}
unsigned int Random::nextUInt() {return (unsigned int) nextInt();}
double Random::nextDouble(int precision) {
    return double(nextUInt() % precision) / double(precision);
}
float Random::nextFloat(int precision) {return float(nextDouble(precision));}

