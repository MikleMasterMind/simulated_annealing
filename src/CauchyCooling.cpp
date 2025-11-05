#include "CauchyCooling.h"

double CauchyCooling::cool(double currentTemperature, int iteration) {
    if (iteration == 0) {
        return currentTemperature;
    }
    return currentTemperature / (1 + iteration);
}