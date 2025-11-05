#pragma once

#include <vector>
#include <memory>
#include "ScheduleSolution.h"

class SolutionGenerator {
public:
    static std::shared_ptr<ScheduleSolution> generateRandomSolution(
        int jobCount, int processorCount, const std::vector<double>& jobDurations);
    
private:
    static std::vector<int> generateRandomAssignment(int jobCount, int processorCount);
};