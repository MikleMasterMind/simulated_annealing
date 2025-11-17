#pragma once

#include "ICoolingLaw.h"

class CauchyCooling : public ICoolingLaw {
public:
    void initialize(double temperature) override;
    double cool(int iteration) override;
private:
    double temperature_;
};