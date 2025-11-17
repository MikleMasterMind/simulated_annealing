#include "BoltzmannCooling.h"
#include <cmath>

double BoltzmannCooling::cool(int iteration) {
    if (iteration == 0) {
        return temperature_;
    }
    return temperature_ / std::log(1 + iteration);
}

void BoltzmannCooling::initialize(double temperature) {
    temperature_ = temperature;
}
