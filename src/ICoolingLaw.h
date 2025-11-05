#pragma once

class ICoolingLaw {
public:
    virtual ~ICoolingLaw() = default;
    virtual double cool(double currentTemperature, int iteration) = 0;
};