#include "CSVDataGenerator.h"
#include <fstream>
#include <random>
#include <chrono>

void CSVDataGenerator::generateData(int jobCount, int processorCount,
                                   double minDuration, double maxDuration,
                                   const std::string& outputPath) {
    
    if (jobCount <= 0 || processorCount <= 0) {
        throw std::invalid_argument("Job count and processor count must be positive");
    }
    
    if (minDuration <= 0 || maxDuration <= minDuration) {
        throw std::invalid_argument("Invalid duration range");
    }
    
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + outputPath);
    }
    
    file << "processor_count,job_count,min_duration,max_duration\n";
    file << processorCount << "," << jobCount << "," << minDuration << "," << maxDuration << "\n";
    
    file << "job_durations\n";
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 generator(static_cast<unsigned int>(seed));
    std::uniform_real_distribution<double> distribution(minDuration, maxDuration);
    
    std::vector<double> durations(jobCount);
    for (int i = 0; i < jobCount; ++i) {
        durations[i] = distribution(generator);
    }
    
    for (size_t i = 0; i < durations.size(); ++i) {
        file << durations[i];
        if (i < durations.size() - 1) {
            file << ",";
        }
    }
    file << "\n";
    
    file.close();
}