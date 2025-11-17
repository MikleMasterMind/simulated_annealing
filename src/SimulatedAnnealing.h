#pragma once

#include <memory>
#include <random>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "ISolution.h"
#include "IMutation.h"
#include "ICoolingLaw.h"

class SimulatedAnnealing {
public:
    SimulatedAnnealing();
    
    void setInitialSolution(const std::shared_ptr<ISolution>& solution);
    void setMutation(const std::shared_ptr<IMutation>& mutation);
    void setCoolingLaw(const std::shared_ptr<ICoolingLaw>& coolingLaw);
    void setInitialTemperature(double temperature);
    void setIterationsPerTemperature(int iterations);
    void setMaxIterationsWithoutImprovement(int iterations);
    
    // Новые методы для многопоточности
    void setCurrentSolution(const std::shared_ptr<ISolution>& solution);
    std::shared_ptr<ISolution> getCurrentSolution() const;
    std::shared_ptr<ISolution> getBestSolution() const;
    double getBestFitness() const;
    
    void stop();
    bool isRunning() const;
    
    std::shared_ptr<ISolution> run();

private:
    std::shared_ptr<ISolution> currentSolution_;
    std::shared_ptr<ISolution> bestSolution_;
    std::shared_ptr<IMutation> mutation_;
    std::shared_ptr<ICoolingLaw> coolingLaw_;
    
    double initialTemperature_;
    double currentTemperature_;
    int iterationsPerTemperature_;
    int maxIterationsWithoutImprovement_;
    
    std::atomic<bool> isRunning_;
    std::atomic<bool> shouldStop_;
    
    mutable std::mt19937 randomGenerator_;
    
    bool shouldAcceptSolution(double deltaF) const;
};