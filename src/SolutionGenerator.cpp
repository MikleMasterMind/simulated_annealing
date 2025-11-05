#include "SolutionGenerator.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <stdexcept>

std::shared_ptr<ScheduleSolution> SolutionGenerator::generateRandomSolution(
    int jobCount, int processorCount, const std::vector<double>& jobDurations) {
    
    auto assignment = generateRandomAssignment(jobCount, processorCount);
    auto solution = std::make_shared<ScheduleSolution>(jobCount, processorCount, jobDurations);
    
    for (int i = 0; i < jobCount; ++i) {
        solution->assignJobToProcessor(i, assignment[i]);
    }
    
    return solution;
}

std::shared_ptr<ScheduleSolution> SolutionGenerator::generateBalancedSolution(
    int jobCount, int processorCount, const std::vector<double>& jobDurations) {
    
    auto assignment = generateBalancedAssignment(jobCount, processorCount, jobDurations);
    auto solution = std::make_shared<ScheduleSolution>(jobCount, processorCount, jobDurations);
    
    for (int i = 0; i < jobCount; ++i) {
        solution->assignJobToProcessor(i, assignment[i]);
    }
    
    return solution;
}

std::shared_ptr<ScheduleSolution> SolutionGenerator::generateWorstCaseSolution(
    int jobCount, int processorCount, const std::vector<double>& jobDurations) {
    
    auto assignment = generateWorstCaseAssignment(jobCount, processorCount);
    auto solution = std::make_shared<ScheduleSolution>(jobCount, processorCount, jobDurations);
    
    for (int i = 0; i < jobCount; ++i) {
        solution->assignJobToProcessor(i, assignment[i]);
    }
    
    return solution;
}

std::vector<int> SolutionGenerator::generateRandomAssignment(int jobCount, int processorCount) {
    if (jobCount <= 0 || processorCount <= 0) {
        throw std::invalid_argument("Job count and processor count must be positive");
    }
    
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 generator(static_cast<unsigned int>(seed));
    std::uniform_int_distribution<int> distribution(0, processorCount - 1);
    
    std::vector<int> assignment(jobCount);
    for (int i = 0; i < jobCount; ++i) {
        assignment[i] = distribution(generator);
    }
    
    return assignment;
}

std::vector<int> SolutionGenerator::generateBalancedAssignment(int jobCount, int processorCount, 
                                                              const std::vector<double>& jobDurations) {
    if (jobCount <= 0 || processorCount <= 0) {
        throw std::invalid_argument("Job count and processor count must be positive");
    }
    
    if (jobDurations.size() != static_cast<size_t>(jobCount)) {
        throw std::invalid_argument("Job durations size must match job count");
    }
    
    std::vector<std::pair<double, int>> jobsWithIndices;
    for (int i = 0; i < jobCount; ++i) {
        jobsWithIndices.emplace_back(jobDurations[i], i);
    }
    
    std::sort(jobsWithIndices.rbegin(), jobsWithIndices.rend());
    
    std::vector<double> processorLoad(processorCount, 0.0);
    std::vector<int> assignment(jobCount);
    
    for (const auto& [duration, jobIndex] : jobsWithIndices) {
        int minLoadProcessor = 0;
        double minLoad = processorLoad[0];
        
        for (int j = 1; j < processorCount; ++j) {
            if (processorLoad[j] < minLoad) {
                minLoad = processorLoad[j];
                minLoadProcessor = j;
            }
        }
        
        assignment[jobIndex] = minLoadProcessor;
        processorLoad[minLoadProcessor] += duration;
    }
    
    return assignment;
}

std::vector<int> SolutionGenerator::generateWorstCaseAssignment(int jobCount, int processorCount) {
    if (jobCount <= 0 || processorCount <= 0) {
        throw std::invalid_argument("Job count and processor count must be positive");
    }
    
    std::vector<int> assignment(jobCount, 0);
    
    if (processorCount > 1) {
        for (int i = 1; i < jobCount; ++i) {
            assignment[i] = 1;
        }
    }
    
    return assignment;
}