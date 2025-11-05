#pragma once

#include <memory>
#include "ISolution.h"

class IMutation {
public:
    virtual ~IMutation() = default;
    virtual std::shared_ptr<ISolution> apply(const std::shared_ptr<ISolution>& solution) = 0;
};