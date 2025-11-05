#pragma once

#include "IDataGenerator.h"
#include <vector>
#include <string>

class CSVDataGenerator : public IDataGenerator {
public:
    void generateData(int jobCount, int processorCount,
                     double minDuration, double maxDuration,
                     const std::string& outputPath) override;

private:
    std::vector<double> generateRandomDurations(int jobCount, double minDuration, double maxDuration);
};