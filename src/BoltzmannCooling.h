#pragma once

#include "ICoolingLaw.h"

class BoltzmannCooling : public ICoolingLaw {
public:
    double cool(double currentTemperature, int iteration) override;
};