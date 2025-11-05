#include "ScheduleSolution.h"
#include <stdexcept>
#include <algorithm>
#include <random>
#include <chrono>

ScheduleSolution::ScheduleSolution(int jobCount, int processorCount, const std::vector<double>& jobDurations)
    : jobCount_(jobCount)
    , processorCount_(processorCount)
    , jobDurations_(jobDurations) {
    
    if (jobCount <= 0 || processorCount <= 0) {
        throw std::invalid_argument("Job count and processor count must be positive");
    }
    
    if (jobDurations.size() != static_cast<size_t>(jobCount)) {
        throw std::invalid_argument("Job durations size must match job count");
    }
    
    initializeAssignmentMatrix();
}

ScheduleSolution::ScheduleSolution(const ScheduleSolution& other)
    : jobCount_(other.jobCount_)
    , processorCount_(other.processorCount_)
    , jobDurations_(other.jobDurations_)
    , assignmentMatrix_(other.assignmentMatrix_) {
}

void ScheduleSolution::initializeAssignmentMatrix() {
    assignmentMatrix_.resize(jobCount_);
    for (int i = 0; i < jobCount_; ++i) {
        assignmentMatrix_[i].resize(processorCount_, false);
    }
}

double ScheduleSolution::evaluate() const {
    // Вычисляем время завершения каждой работы
    std::vector<double> completionTimes(jobCount_, 0.0);
    
    // Для каждого процессора вычисляем время завершения его работ
    for (int j = 0; j < processorCount_; ++j) {
        double processorTime = 0.0;
        for (int i = 0; i < jobCount_; ++i) {
            if (assignmentMatrix_[i][j]) {
                processorTime += jobDurations_[i];
                completionTimes[i] = processorTime;
            }
        }
    }
    
    double maxCompletionTime = *std::max_element(completionTimes.begin(), completionTimes.end());
    double minCompletionTime = *std::min_element(completionTimes.begin(), completionTimes.end());
    
    return maxCompletionTime - minCompletionTime;  // критерий K1 - разбалансированность
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

const std::vector<std::vector<bool>>& ScheduleSolution::getAssignmentMatrix() const {
    return assignmentMatrix_;
}

void ScheduleSolution::assignJobToProcessor(int jobIndex, int processorIndex) {
    validateIndices(jobIndex, processorIndex);
    
    // Снимаем назначение со всех процессоров
    for (int j = 0; j < processorCount_; ++j) {
        assignmentMatrix_[jobIndex][j] = false;
    }
    
    // Назначаем на целевой процессор
    assignmentMatrix_[jobIndex][processorIndex] = true;
}

bool ScheduleSolution::isJobAssignedToProcessor(int jobIndex, int processorIndex) const {
    validateIndices(jobIndex, processorIndex);
    return assignmentMatrix_[jobIndex][processorIndex];
}

int ScheduleSolution::getJobProcessor(int jobIndex) const {
    validateIndices(jobIndex, 0);  // проверяем только jobIndex
    
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

std::shared_ptr<ScheduleSolution> ScheduleSolution::createRandomSolution(
    int jobCount, int processorCount, const std::vector<double>& jobDurations) {
    
    auto solution = std::make_shared<ScheduleSolution>(jobCount, processorCount, jobDurations);
    
    // Инициализируем генератор случайных чисел
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 generator(static_cast<unsigned int>(seed));
    std::uniform_int_distribution<int> distribution(0, processorCount - 1);
    
    // Случайно распределяем работы по процессорам
    for (int i = 0; i < jobCount; ++i) {
        int randomProcessor = distribution(generator);
        solution->assignJobToProcessor(i, randomProcessor);
    }
    
    return solution;
}