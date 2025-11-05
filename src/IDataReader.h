#pragma once

#include <vector>
#include <string>

struct InputData {
    int processorCount;
    int jobCount;
    double minDuration;
    double maxDuration;
    std::vector<double> jobDurations;
};

class IDataReader {
public:
    virtual ~IDataReader() = default;
    virtual InputData readData(const std::string& inputPath) = 0;
};