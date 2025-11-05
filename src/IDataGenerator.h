#pragma once

#include <vector>
#include <string>

class IDataGenerator {
public:
    virtual ~IDataGenerator() = default;
    virtual void generateData(int jobCount, int processorCount, 
                             double minDuration, double maxDuration,
                             const std::string& outputPath) = 0;
};