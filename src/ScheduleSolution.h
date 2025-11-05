#pragma once

#include <vector>
#include <memory>
#include "ISolution.h"

class ScheduleSolution : public ISolution {
public:
    ScheduleSolution(int jobCount, int processorCount, const std::vector<double>& jobDurations);
    ScheduleSolution(const ScheduleSolution& other);
    
    double evaluate() const override;
    std::shared_ptr<ISolution> clone() const override;
    
    int getJobCount() const;
    int getProcessorCount() const;
    const std::vector<double>& getJobDurations() const;
    const std::vector<std::vector<bool>>& getAssignmentMatrix() const;
    
    void assignJobToProcessor(int jobIndex, int processorIndex);
    bool isJobAssignedToProcessor(int jobIndex, int processorIndex) const;
    int getJobProcessor(int jobIndex) const;
    
    static std::shared_ptr<ScheduleSolution> createRandomSolution(
        int jobCount, int processorCount, const std::vector<double>& jobDurations);

private:
    int jobCount_;
    int processorCount_;
    std::vector<double> jobDurations_;
    std::vector<std::vector<bool>> assignmentMatrix_;    

    void validateIndices(int jobIndex, int processorIndex) const;
    void initializeAssignmentMatrix();
};