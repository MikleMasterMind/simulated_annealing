#include "ScheduleSolution.h"
#include <stdexcept>
#include <algorithm>
#include <random>
#include <chrono>

ScheduleSolution::ScheduleSolution(int jobCount, int processorCount, const std::vector<double>& jobDurations)
    : jobCount_(jobCount)
    , processorCount_(processorCount)
    , jobDurations_(jobDurations) 
    {
    assignmentMatrix_.resize(jobCount_);
    for (int i = 0; i < jobCount_; ++i) {
        assignmentMatrix_[i].resize(processorCount_, false);
    }
}

ScheduleSolution::ScheduleSolution(const ScheduleSolution& other)
    : jobCount_(other.jobCount_)
    , processorCount_(other.processorCount_)
    , jobDurations_(other.jobDurations_)
    , assignmentMatrix_(other.assignmentMatrix_) {
}

double ScheduleSolution::evaluate() const {
    std::vector<double> completionTimes(jobCount_ * processorCount_);
    
    for (int j = 0; j < processorCount_; ++j) {
        double processorTime = 0.0;
        for (int i = 0; i < jobCount_; ++i) {
            if (assignmentMatrix_[i][j]) {
                processorTime += jobDurations_[i];
                completionTimes[j * jobCount_ + i] = processorTime;
            }
        }
    }
    
    double maxCompletionTime = *std::max_element(completionTimes.begin(), completionTimes.end());
    double minCompletionTime = *std::min_element(completionTimes.begin(), completionTimes.end());
    
    return maxCompletionTime - minCompletionTime;
}

std::shared_ptr<ISolution> ScheduleSolution::clone() const {
    return std::make_shared<ScheduleSolution>(*this);
}

int ScheduleSolution::getJobCount() const {
    return jobCount_;
}

int ScheduleSolution::getProcessorCount() const {
    return processorCount_;
}

const std::vector<double>& ScheduleSolution::getJobDurations() const {
    return jobDurations_;
}

void ScheduleSolution::assignJobToProcessor(int jobIndex, int processorIndex) {
    validateIndices(jobIndex, processorIndex);
    
    for (int j = 0; j < processorCount_; ++j) {
        assignmentMatrix_[jobIndex][j] = false;
    }
    
    assignmentMatrix_[jobIndex][processorIndex] = true;
}

bool ScheduleSolution::isJobAssignedToProcessor(int jobIndex, int processorIndex) const {
    validateIndices(jobIndex, processorIndex);
    return assignmentMatrix_[jobIndex][processorIndex];
}

int ScheduleSolution::getJobProcessor(int jobIndex) const {
    validateIndices(jobIndex, 0);
    
    for (int j = 0; j < processorCount_; ++j) {
        if (assignmentMatrix_[jobIndex][j]) {
            return j;
        }
    }
    
    throw std::runtime_error("Job is not assigned to any processor");
}

void ScheduleSolution::validateIndices(int jobIndex, int processorIndex) const {
    if (jobIndex < 0 || jobIndex >= jobCount_) {
        throw std::out_of_range("Job index out of range");
    }
    
    if (processorIndex < 0 || processorIndex >= processorCount_) {
        throw std::out_of_range("Processor index out of range");
    }
}
