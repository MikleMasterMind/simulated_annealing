#include "ScheduleMutation.h"
#include <stdexcept>
#include <chrono>
#include <algorithm>

ScheduleMutation::ScheduleMutation()
    : moveProbability_(0.7)
    , swapProbability_(0.3) {
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    randomGenerator_.seed(static_cast<unsigned int>(seed));
}

std::shared_ptr<ISolution> ScheduleMutation::apply(const std::shared_ptr<ISolution>& solution) {
    auto scheduleSolution = std::dynamic_pointer_cast<ScheduleSolution>(solution);
    if (!scheduleSolution) {
        throw std::invalid_argument("ScheduleMutation can only work with ScheduleSolution");
    }
    
    double totalProbability = moveProbability_ + swapProbability_;
    if (std::abs(totalProbability - 1.0) > 1e-6) {
        throw std::runtime_error("Mutation probabilities must sum to 1.0");
    }
    
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    double randomValue = distribution(randomGenerator_);
    
    if (randomValue < moveProbability_) {
        return applyMoveOperation(scheduleSolution);
    } else {
        return applySwapOperation(scheduleSolution);
    }
}

void ScheduleMutation::setMoveProbability(double probability) {
    if (probability < 0.0 || probability > 1.0) {
        throw std::invalid_argument("Probability must be between 0.0 and 1.0");
    }
    moveProbability_ = probability;
    swapProbability_ = 1.0 - probability;
}

void ScheduleMutation::setSwapProbability(double probability) {
    if (probability < 0.0 || probability > 1.0) {
        throw std::invalid_argument("Probability must be between 0.0 and 1.0");
    }
    swapProbability_ = probability;
    moveProbability_ = 1.0 - probability;
}

std::shared_ptr<ScheduleSolution> ScheduleMutation::applyMoveOperation(
    const std::shared_ptr<ScheduleSolution>& solution) {
    
    auto newSolution = std::make_shared<ScheduleSolution>(*solution);
    
    int jobIndex = selectRandomJob(newSolution);
    
    int currentProcessor = newSolution->getJobProcessor(jobIndex);
    
    int newProcessor = selectRandomProcessorExcept(newSolution, currentProcessor);
    
    newSolution->assignJobToProcessor(jobIndex, newProcessor);
    
    return newSolution;
}

std::shared_ptr<ScheduleSolution> ScheduleMutation::applySwapOperation(
    const std::shared_ptr<ScheduleSolution>& solution) {
    
    auto newSolution = std::make_shared<ScheduleSolution>(*solution);
    
    auto [job1, job2] = selectTwoJobsOnDifferentProcessors(newSolution);
    
    if (job1 == -1 || job2 == -1) {
        return applyMoveOperation(newSolution);
    }
    
    int processor1 = newSolution->getJobProcessor(job1);
    int processor2 = newSolution->getJobProcessor(job2);
    
    newSolution->assignJobToProcessor(job1, processor2);
    newSolution->assignJobToProcessor(job2, processor1);
    
    return newSolution;
}

int ScheduleMutation::selectRandomJob(const std::shared_ptr<ScheduleSolution>& solution) const {
    std::uniform_int_distribution<int> distribution(0, solution->getJobCount() - 1);
    return distribution(randomGenerator_);
}

int ScheduleMutation::selectRandomProcessorExcept(
    const std::shared_ptr<ScheduleSolution>& solution, int excludedProcessor) const {
    
    int processorCount = solution->getProcessorCount();
    if (processorCount <= 1) {
        throw std::runtime_error("Cannot select different processor when processor count <= 1");
    }
    
    std::vector<int> availableProcessors;
    for (int i = 0; i < processorCount; ++i) {
        if (i != excludedProcessor) {
            availableProcessors.push_back(i);
        }
    }
    
    std::uniform_int_distribution<int> distribution(0, availableProcessors.size() - 1);
    return availableProcessors[distribution(randomGenerator_)];
}

std::pair<int, int> ScheduleMutation::selectTwoJobsOnDifferentProcessors(
    const std::shared_ptr<ScheduleSolution>& solution) const {
    
    int jobCount = solution->getJobCount();
    int processorCount = solution->getProcessorCount();
    
    if (jobCount < 2 || processorCount < 2) {
        return {-1, -1};
    }
    
    std::vector<std::vector<int>> jobsByProcessor(processorCount);
    for (int i = 0; i < jobCount; ++i) {
        int processor = solution->getJobProcessor(i);
        jobsByProcessor[processor].push_back(i);
    }
    
    std::vector<int> nonEmptyProcessors;
    for (int i = 0; i < processorCount; ++i) {
        if (!jobsByProcessor[i].empty()) {
            nonEmptyProcessors.push_back(i);
        }
    }
    
    if (nonEmptyProcessors.size() < 2) {
        return {-1, -1};
    }
    
    std::uniform_int_distribution<int> processorDist(0, nonEmptyProcessors.size() - 1);
    int processor1Index = processorDist(randomGenerator_);
    int processor2Index;
    do {
        processor2Index = processorDist(randomGenerator_);
    } while (processor2Index == processor1Index);
    
    int processor1 = nonEmptyProcessors[processor1Index];
    int processor2 = nonEmptyProcessors[processor2Index];
    
    std::uniform_int_distribution<int> jobDist1(0, jobsByProcessor[processor1].size() - 1);
    std::uniform_int_distribution<int> jobDist2(0, jobsByProcessor[processor2].size() - 1);
    
    int job1 = jobsByProcessor[processor1][jobDist1(randomGenerator_)];
    int job2 = jobsByProcessor[processor2][jobDist2(randomGenerator_)];
    
    return {job1, job2};
}