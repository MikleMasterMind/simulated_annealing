#pragma once

#include "ICoolingLaw.h"

class LogarithmicCooling : public ICoolingLaw {
public:
    double cool(double currentTemperature, int iteration) override;
};