#include "LogarithmicCooling.h"
#include <cmath>

double LogarithmicCooling::cool(double currentTemperature, int iteration) {
    if (iteration == 0) {
        return currentTemperature;
    }
    return currentTemperature * std::log(1 + iteration) / (1 + iteration);
}