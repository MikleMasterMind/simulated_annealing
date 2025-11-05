#include "BoltzmannCooling.h"
#include <cmath>

double BoltzmannCooling::cool(double currentTemperature, int iteration) {
    if (iteration == 0) {
        return currentTemperature;
    }
    return currentTemperature / std::log(1 + iteration);
}