#pragma once

#include <memory>
#include <random>
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
    
    std::mt19937 randomGenerator_;
    
    void initializeRandomGenerator();
    bool shouldAcceptWorseSolution(double deltaF);
};