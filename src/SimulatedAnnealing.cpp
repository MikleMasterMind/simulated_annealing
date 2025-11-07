#include "SimulatedAnnealing.h"
#include <cmath>
#include <chrono>
#include <thread>

SimulatedAnnealing::SimulatedAnnealing()
    : initialTemperature_(1000.0)
    , currentTemperature_(1000.0)
    , iterationsPerTemperature_(100)
    , maxIterationsWithoutImprovement_(100)
    , isRunning_(false)
    , isPaused_(false)
    , shouldStop_(false) {
    initializeRandomGenerator();
}

void SimulatedAnnealing::setInitialSolution(const std::shared_ptr<ISolution>& solution) {
    std::lock_guard<std::mutex> lock(solutionMutex_);
    currentSolution_ = solution->clone();
    bestSolution_ = solution->clone();
}

void SimulatedAnnealing::setMutation(const std::shared_ptr<IMutation>& mutation) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    mutation_ = mutation;
}

void SimulatedAnnealing::setCoolingLaw(const std::shared_ptr<ICoolingLaw>& coolingLaw) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    coolingLaw_ = coolingLaw;
}

void SimulatedAnnealing::setInitialTemperature(double temperature) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    initialTemperature_ = temperature;
    currentTemperature_ = temperature;
}

void SimulatedAnnealing::setIterationsPerTemperature(int iterations) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    iterationsPerTemperature_ = iterations;
}

void SimulatedAnnealing::setMaxIterationsWithoutImprovement(int iterations) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    maxIterationsWithoutImprovement_ = iterations;
}

void SimulatedAnnealing::setCurrentSolution(const std::shared_ptr<ISolution>& solution) {
    std::lock_guard<std::mutex> lock(solutionMutex_);
    if (solution) {
        currentSolution_ = solution->clone();
        
        double newFitness = solution->evaluate();
        double bestFitness = bestSolution_->evaluate();
        
        if (newFitness < bestFitness) {
            bestSolution_ = solution->clone();
        }
    }
}

std::shared_ptr<ISolution> SimulatedAnnealing::getCurrentSolution() const {
    std::lock_guard<std::mutex> lock(solutionMutex_);
    return currentSolution_->clone();
}

std::shared_ptr<ISolution> SimulatedAnnealing::getBestSolution() const {
    std::lock_guard<std::mutex> lock(solutionMutex_);
    return bestSolution_->clone();
}

double SimulatedAnnealing::getBestFitness() const {
    std::lock_guard<std::mutex> lock(solutionMutex_);
    return bestSolution_->evaluate();
}

void SimulatedAnnealing::pause() {
    isPaused_ = true;
}

void SimulatedAnnealing::resume() {
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        isPaused_ = false;
    }
    pauseCondition_.notify_all();
}

void SimulatedAnnealing::stop() {
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        shouldStop_ = true;
        isPaused_ = false;
    }
    pauseCondition_.notify_all();
}

bool SimulatedAnnealing::isRunning() const {
    return isRunning_;
}

void SimulatedAnnealing::initializeRandomGenerator() {
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    randomGenerator_.seed(static_cast<unsigned int>(seed));
}

bool SimulatedAnnealing::shouldAcceptWorseSolution(double deltaF) const {
    if (deltaF <= 0) {
        return true;
    }
    
    double probability = std::exp(-deltaF / currentTemperature_);
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(randomGenerator_) < probability;
}

void SimulatedAnnealing::waitIfPaused() {
    std::unique_lock<std::mutex> lock(stateMutex_);
    pauseCondition_.wait(lock, [this]() {
        return !isPaused_ || shouldStop_;
    });
}

std::shared_ptr<ISolution> SimulatedAnnealing::run() {
    if (!currentSolution_ || !mutation_ || !coolingLaw_) {
        return nullptr;
    }
    
    isRunning_ = true;
    shouldStop_ = false;
    isPaused_ = false;
    
    int iterationsWithoutImprovement = 0;
    int totalIteration = 0;
    
    {
        std::lock_guard<std::mutex> lock(solutionMutex_);
        bestSolution_ = currentSolution_->clone();
    }
    
    double bestFitness = bestSolution_->evaluate();
    
    while (iterationsWithoutImprovement < maxIterationsWithoutImprovement_ && !shouldStop_) {
        waitIfPaused();
        
        if (shouldStop_) break;
        
        bool improvedInThisCycle = false;
        
        for (int i = 0; i < iterationsPerTemperature_ && !shouldStop_; ++i) {
            waitIfPaused();
            if (shouldStop_) break;
            
            auto newSolution = mutation_->apply(currentSolution_);
            
            double currentFitness, newFitness;
            {
                std::lock_guard<std::mutex> lock(solutionMutex_);
                currentFitness = currentSolution_->evaluate();
                newFitness = newSolution->evaluate();
            }
            
            double deltaF = newFitness - currentFitness;
            
            if (shouldAcceptWorseSolution(deltaF)) {
                std::lock_guard<std::mutex> lock(solutionMutex_);
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
        
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            currentTemperature_ = coolingLaw_->cool(currentTemperature_, totalIteration);
        }
        
        if (currentTemperature_ < 1e-10) {
            break;
        }
    }
    
    isRunning_ = false;
    
    std::lock_guard<std::mutex> lock(solutionMutex_);
    return bestSolution_;
}