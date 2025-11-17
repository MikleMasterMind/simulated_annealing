#include "CauchyCooling.h"

double CauchyCooling::cool(int iteration) {
    if (iteration == 0) {
        return temperature_;
    }
    return temperature_ / (1 + iteration);
}

void CauchyCooling::initialize(double temperature) {
    temperature_ = temperature;
}
