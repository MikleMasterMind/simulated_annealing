#pragma once

#include <memory>

class ISolution {
public:
    virtual ~ISolution() = default;
    virtual double evaluate() const = 0;
    virtual std::shared_ptr<ISolution> clone() const = 0;
};