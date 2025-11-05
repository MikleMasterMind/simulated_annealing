#pragma once

#include "ICoolingLaw.h"

class CauchyCooling : public ICoolingLaw {
public:
    double cool(double currentTemperature, int iteration) override;
};