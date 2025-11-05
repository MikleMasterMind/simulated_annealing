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
    
    // Пропускаем заголовок
    std::getline(file, line);
    
    // Читаем основные параметры
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
    
    // Пропускаем заголовок длительностей
    std::getline(file, line);
    
    // Читаем длительности работ
    if (std::getline(file, line)) {
        data.jobDurations = parseDurations(line);
    }
    
    file.close();
    
    validateData(data);
    return data;
}

std::vector<double> CSVDataReader::parseDurations(const std::string& line) {
    std::vector<double> durations;
    std::stringstream ss(line);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        durations.push_back(std::stod(token));
    }
    
    return durations;
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