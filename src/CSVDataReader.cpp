#include "CSVDataReader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

InputData CSVDataReader::readData(const std::string& inputPath) {
    std::ifstream file(inputPath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + inputPath);
    }
    
    InputData data;
    std::string line;
    
    std::getline(file, line);
    
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        
        std::getline(ss, token, ',');
        data.processorCount = std::stoi(token);
        
        std::getline(ss, token, ',');
        data.jobCount = std::stoi(token);
        
        std::getline(ss, token, ',');
        data.minDuration = std::stod(token);
        
        std::getline(ss, token, ',');
        data.maxDuration = std::stod(token);
    }
    
    std::getline(file, line);
    
    if (std::getline(file, line)) {
        std::vector<double> durations;
        std::stringstream ss(line);
        std::string token;
        
        while (std::getline(ss, token, ',')) {
            data.jobDurations.push_back(std::stod(token));
        }
    }
    
    file.close();
    
    validateData(data);
    return data;
}

void CSVDataReader::validateData(const InputData& data) {
    if (data.processorCount <= 0) {
        throw std::runtime_error("Processor count must be positive");
    }
    
    if (data.jobCount <= 0) {
        throw std::runtime_error("Job count must be positive");
    }
    
    if (data.minDuration <= 0 || data.maxDuration <= data.minDuration) {
        throw std::runtime_error("Invalid duration range");
    }
    
    if (data.jobDurations.size() != static_cast<size_t>(data.jobCount)) {
        throw std::runtime_error("Job durations count doesn't match job count");
    }
    
    for (double duration : data.jobDurations) {
        if (duration < data.minDuration || duration > data.maxDuration) {
            throw std::runtime_error("Job duration out of specified range");
        }
    }
}