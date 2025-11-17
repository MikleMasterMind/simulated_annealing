#pragma once

#include "IDataReader.h"
#include <vector>
#include <string>

class CSVDataReader : public IDataReader {
public:
    InputData readData(const std::string& inputPath) override;

private:
    void validateData(const InputData& data);
};