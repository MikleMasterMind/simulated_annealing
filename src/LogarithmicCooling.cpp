#include "LogarithmicCooling.h"
#include <cmath>

double LogarithmicCooling::cool(int iteration) {
    if (iteration == 0) {
        return temperature_;
    }
    return temperature_ * std::log(1 + iteration) / (1 + iteration);
}

void LogarithmicCooling::initialize(double temperature) {
    temperature_ = temperature;
}
