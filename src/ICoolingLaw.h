#pragma once

class ICoolingLaw {
public:
    virtual ~ICoolingLaw() = default;
    virtual void initialize(double temperature) = 0;
    virtual double cool(int iteration) = 0;
};