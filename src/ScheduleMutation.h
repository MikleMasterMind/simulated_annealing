#pragma once

#include <memory>
#include <random>
#include "IMutation.h"
#include "ScheduleSolution.h"

class ScheduleMutation : public IMutation {
public:
    ScheduleMutation();
    
    std::shared_ptr<ISolution> apply(const std::shared_ptr<ISolution>& solution) override;
    
    void setMoveProbability(double probability);
    void setSwapProbability(double probability);
    
    std::shared_ptr<ScheduleSolution> applyMoveOperation(
        const std::shared_ptr<ScheduleSolution>& solution);
    std::shared_ptr<ScheduleSolution> applySwapOperation(
        const std::shared_ptr<ScheduleSolution>& solution);

private:
    double moveProbability_;
    double swapProbability_;
    
    mutable std::mt19937 randomGenerator_;
    
    int selectRandomJob(const std::shared_ptr<ScheduleSolution>& solution) const;
    int selectRandomProcessorExcept(const std::shared_ptr<ScheduleSolution>& solution, int excludedProcessor) const;
    std::pair<int, int> selectTwoJobsOnDifferentProcessors(
        const std::shared_ptr<ScheduleSolution>& solution) const;
};