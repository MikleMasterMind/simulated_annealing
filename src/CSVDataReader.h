#pragma once

#include "IDataReader.h"
#include <vector>
#include <string>

class CSVDataReader : public IDataReader {
public:
    InputData readData(const std::string& inputPath) override;

private:
    std::vector<double> parseDurations(const std::string& line);
    void validateData(const InputData& data);
};