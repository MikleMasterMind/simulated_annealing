#include "SimulatedAnnealing.h"
#include <cmath>
#include <chrono>

SimulatedAnnealing::SimulatedAnnealing()
    : initialTemperature_(1000.0)
    , currentTemperature_(1000.0)
    , iterationsPerTemperature_(100)
    , maxIterationsWithoutImprovement_(100) {
    initializeRandomGenerator();
}

void SimulatedAnnealing::setInitialSolution(const std::shared_ptr<ISolution>& solution) {
    currentSolution_ = solution->clone();
    bestSolution_ = solution->clone();
}

void SimulatedAnnealing::setMutation(const std::shared_ptr<IMutation>& mutation) {
    mutation_ = mutation;
}

void SimulatedAnnealing::setCoolingLaw(const std::shared_ptr<ICoolingLaw>& coolingLaw) {
    coolingLaw_ = coolingLaw;
}

void SimulatedAnnealing::setInitialTemperature(double temperature) {
    initialTemperature_ = temperature;
    currentTemperature_ = temperature;
}

void SimulatedAnnealing::setIterationsPerTemperature(int iterations) {
    iterationsPerTemperature_ = iterations;
}

void SimulatedAnnealing::setMaxIterationsWithoutImprovement(int iterations) {
    maxIterationsWithoutImprovement_ = iterations;
}

void SimulatedAnnealing::initializeRandomGenerator() {
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    randomGenerator_.seed(static_cast<unsigned int>(seed));
}

bool SimulatedAnnealing::shouldAcceptWorseSolution(double deltaF) {
    if (deltaF <= 0) {
        return true;
    }
    
    double probability = std::exp(-deltaF / currentTemperature_);
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(randomGenerator_) < probability;
}

std::shared_ptr<ISolution> SimulatedAnnealing::run() {
    if (!currentSolution_ || !mutation_ || !coolingLaw_) {
        return nullptr;
    }
    
    int iterationsWithoutImprovement = 0;
    int totalIteration = 0;
    double bestFitness = bestSolution_->evaluate();
    
    while (iterationsWithoutImprovement < maxIterationsWithoutImprovement_) {
        bool improvedInThisCycle = false;
        
        for (int i = 0; i < iterationsPerTemperature_; ++i) {
            auto newSolution = mutation_->apply(currentSolution_);
            double currentFitness = currentSolution_->evaluate();
            double newFitness = newSolution->evaluate();
            double deltaF = newFitness - currentFitness;
            
            if (shouldAcceptWorseSolution(deltaF)) {
                currentSolution_ = newSolution;
                
                if (newFitness < bestFitness) {
                    bestSolution_ = newSolution->clone();
                    bestFitness = newFitness;
                    improvedInThisCycle = true;
                }
            }
            
            ++totalIteration;
        }
        
        if (improvedInThisCycle) {
            iterationsWithoutImprovement = 0;
        } else {
            ++iterationsWithoutImprovement;
        }
        
        currentTemperature_ = coolingLaw_->cool(currentTemperature_, totalIteration);
        
        if (currentTemperature_ < 1e-10) {
            break;
        }
    }
    
    return bestSolution_;
}