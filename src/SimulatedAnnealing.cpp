#include "SimulatedAnnealing.h"
#include "Logger.h"
#include <cmath>
#include <chrono>
#include <thread>

SimulatedAnnealing::SimulatedAnnealing()
    : initialTemperature_(0.0)
    , currentTemperature_(0.0)
    , iterationsPerTemperature_(0)
    , maxIterationsWithoutImprovement_(0)
    , isRunning_(false)
    , isPaused_(false)
    , shouldStop_(false) {
    initializeRandomGenerator();
}

void SimulatedAnnealing::setInitialSolution(const std::shared_ptr<ISolution>& solution) {
    std::lock_guard<std::mutex> lock(solutionMutex_);
    currentSolution_ = solution->clone();
    bestSolution_ = solution->clone();
    Logger::log("Initial solution set, fitness: " + std::to_string(solution->evaluate()));
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
    Logger::log("Initial temperature set to: " + std::to_string(temperature));
}

void SimulatedAnnealing::setIterationsPerTemperature(int iterations) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    iterationsPerTemperature_ = iterations;
    Logger::log("Iterations per temperature set to: " + std::to_string(iterations));
}

void SimulatedAnnealing::setMaxIterationsWithoutImprovement(int iterations) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    maxIterationsWithoutImprovement_ = iterations;
    Logger::log("Max iterations without improvement set to: " + std::to_string(iterations));
}

void SimulatedAnnealing::setCurrentSolution(const std::shared_ptr<ISolution>& solution) {
    std::lock_guard<std::mutex> lock(solutionMutex_);
    if (solution) {
        currentSolution_ = solution->clone();
        
        double newFitness = solution->evaluate();
        double bestFitness = bestSolution_->evaluate();
        
        Logger::log("External solution set, fitness: " + std::to_string(newFitness));
        
        if (newFitness < bestFitness) {
            bestSolution_ = solution->clone();
            Logger::log("NEW GLOBAL BEST from external: " + std::to_string(newFitness));
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
    Logger::log("Algorithm paused");
}

void SimulatedAnnealing::resume() {
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        isPaused_ = false;
    }
    pauseCondition_.notify_all();
    Logger::log("Algorithm resumed");
}

void SimulatedAnnealing::stop() {
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        shouldStop_ = true;
        isPaused_ = false;
    }
    pauseCondition_.notify_all();
    Logger::log("Algorithm stopped");
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
        Logger::log("ACCEPT: Improvement deltaF=" + std::to_string(deltaF));
        return true;
    }
    
    double probability = std::exp(-deltaF / currentTemperature_);
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    double randomValue = distribution(randomGenerator_);
    bool accepted = randomValue < probability;
    
    Logger::log("DECISION: deltaF=" + std::to_string(deltaF) + 
                ", T=" + std::to_string(currentTemperature_) +
                ", probability=" + std::to_string(probability) +
                ", random=" + std::to_string(randomValue) +
                ", accepted=" + std::to_string(accepted));
    
    return accepted;
}

void SimulatedAnnealing::waitIfPaused() {
    std::unique_lock<std::mutex> lock(stateMutex_);
    pauseCondition_.wait(lock, [this]() {
        return !isPaused_ || shouldStop_;
    });
}

std::shared_ptr<ISolution> SimulatedAnnealing::run() {
    if (!currentSolution_ || !mutation_ || !coolingLaw_) {
        Logger::log("ERROR: Algorithm not properly initialized");
        return nullptr;
    }
    
    isRunning_ = true;
    shouldStop_ = false;
    isPaused_ = false;
    
    int iterationsWithoutImprovement = 0;
    int totalIteration = 0;
    
    double initialFitness;
    {
        std::lock_guard<std::mutex> lock(solutionMutex_);
        bestSolution_ = currentSolution_->clone();
        initialFitness = bestSolution_->evaluate();
    }
    
    double bestFitness = initialFitness;
    
    Logger::log("Algorithm STARTED: T0=" + std::to_string(initialTemperature_) +
                ", iterations_per_temp=" + std::to_string(iterationsPerTemperature_) +
                ", max_no_improve=" + std::to_string(maxIterationsWithoutImprovement_) +
                ", initial_fitness=" + std::to_string(initialFitness));
    
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
                    
                    Logger::log("NEW BEST: fitness improved to " + std::to_string(newFitness) +
                                " (iteration " + std::to_string(totalIteration) + ")");
                }
            }
            
            ++totalIteration;
            
            // Логируем прогресс каждые 100 итераций
            if (totalIteration % 100 == 0) {
                Logger::log("Progress: iteration=" + std::to_string(totalIteration) +
                            ", current_fitness=" + std::to_string(currentFitness) +
                            ", best_fitness=" + std::to_string(bestFitness) +
                            ", T=" + std::to_string(currentTemperature_));
            }
        }
        
        if (improvedInThisCycle) {
            iterationsWithoutImprovement = 0;
            Logger::log("Temperature cycle: IMPROVEMENT found");
        } else {
            ++iterationsWithoutImprovement;
            Logger::log("Temperature cycle: NO improvement, count=" + 
                        std::to_string(iterationsWithoutImprovement));
        }
        
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            double oldTemperature = currentTemperature_;
            currentTemperature_ = coolingLaw_->cool(totalIteration);
            Logger::log("Temperature cooled: " + std::to_string(oldTemperature) + 
                        " -> " + std::to_string(currentTemperature_));
        }
        
        if (currentTemperature_ < 1e-10) {
            Logger::log("Stopping: temperature below threshold");
            break;
        }
    }
    
    isRunning_ = false;
    
    std::lock_guard<std::mutex> lock(solutionMutex_);
    Logger::log("Algorithm FINISHED: total_iterations=" + std::to_string(totalIteration) +
                ", final_fitness=" + std::to_string(bestFitness) +
                ", improvement=" + std::to_string(initialFitness - bestFitness));
    
    return bestSolution_;
}