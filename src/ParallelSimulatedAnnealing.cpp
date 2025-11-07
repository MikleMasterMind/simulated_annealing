#include "ParallelSimulatedAnnealing.h"
#include <chrono>
#include <random>
#include <algorithm>
#include <thread>

ParallelSimulatedAnnealing::ParallelSimulatedAnnealing(int numThreads)
    : numThreads_(numThreads)
    , exchangeInterval_(100)
    , shouldStop_(false)
    , iterationsWithoutImprovement_(0)
    , globalBestFitness_(std::numeric_limits<double>::max())
    , initialTemperature_(1000.0)
    , iterationsPerTemperature_(100)
    , maxIterationsWithoutImprovement_(100) {
    
    if (numThreads_ <= 0) {
        numThreads_ = std::thread::hardware_concurrency();
        if (numThreads_ == 0) numThreads_ = 1;
    }
}

ParallelSimulatedAnnealing::~ParallelSimulatedAnnealing() {
    stop();
}

void ParallelSimulatedAnnealing::setInitialSolution(const std::shared_ptr<ISolution>& solution) {
    std::lock_guard<std::mutex> lock(initialSolutionMutex_);
    initialSolutionTemplate_ = solution;
}

void ParallelSimulatedAnnealing::setMutation(const std::shared_ptr<IMutation>& mutation) {
    std::lock_guard<std::mutex> lock(configMutex_);
    mutation_ = mutation;
}

void ParallelSimulatedAnnealing::setCoolingLaw(const std::shared_ptr<ICoolingLaw>& coolingLaw) {
    std::lock_guard<std::mutex> lock(configMutex_);
    coolingLaw_ = coolingLaw;
}

void ParallelSimulatedAnnealing::setInitialTemperature(double temperature) {
    std::lock_guard<std::mutex> lock(configMutex_);
    initialTemperature_ = temperature;
}

void ParallelSimulatedAnnealing::setIterationsPerTemperature(int iterations) {
    std::lock_guard<std::mutex> lock(configMutex_);
    iterationsPerTemperature_ = iterations;
}

void ParallelSimulatedAnnealing::setMaxIterationsWithoutImprovement(int iterations) {
    std::lock_guard<std::mutex> lock(configMutex_);
    maxIterationsWithoutImprovement_ = iterations;
}

void ParallelSimulatedAnnealing::setExchangeInterval(int interval) {
    std::lock_guard<std::mutex> lock(configMutex_);
    exchangeInterval_ = interval;
}

std::shared_ptr<ISolution> ParallelSimulatedAnnealing::run() {
    if (!initialSolutionTemplate_ || !mutation_ || !coolingLaw_) {
        return nullptr;
    }
    
    globalBestSolution_ = initialSolutionTemplate_->clone();
    globalBestFitness_ = globalBestSolution_->evaluate();
    
    iterationsWithoutImprovement_ = 0;
    shouldStop_ = false;
    
    initializeThreads();
    
    int globalCycle = 0;
    
    while (iterationsWithoutImprovement_ < 10 && !shouldStop_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        bool improved = exchangeSolutions();
        
        if (improved) {
            iterationsWithoutImprovement_ = 0;
        } else {
            iterationsWithoutImprovement_++;
        }
        
        globalCycle++;
        
        if (globalCycle % 10 == 0) {
            bool anyThreadRunning = false;
            for (auto& threadData : threads_) {
                if (threadData.algorithm && threadData.algorithm->isRunning()) {
                    anyThreadRunning = true;
                    break;
                }
            }
            
            if (!anyThreadRunning) {
                break;
            }
        }
    }
    
    stop();
    
    return globalBestSolution_;
}

void ParallelSimulatedAnnealing::workerThread(int threadId) {
    auto& threadData = threads_[threadId];
    
    threadData.algorithm = std::make_unique<SimulatedAnnealing>();
    threadData.algorithm->setInitialSolution(createThreadSpecificSolution());
    threadData.algorithm->setMutation(mutation_);
    threadData.algorithm->setCoolingLaw(coolingLaw_);
    threadData.algorithm->setInitialTemperature(initialTemperature_);
    threadData.algorithm->setIterationsPerTemperature(iterationsPerTemperature_);
    threadData.algorithm->setMaxIterationsWithoutImprovement(maxIterationsWithoutImprovement_);
    
    int localIterations = 0;
    
    while (!shouldStop_ && localIterations < exchangeInterval_ * 10) {
        auto localBest = threadData.algorithm->run();
        
        if (localBest) {
            double localFitness = localBest->evaluate();
            
            std::lock_guard<std::mutex> lock(threadData.solutionMutex);
            threadData.bestSolution = localBest;
            threadData.bestFitness = localFitness;
        }
        
        localIterations += exchangeInterval_;
        
        if (shouldStop_) {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void ParallelSimulatedAnnealing::initializeThreads() {
    threads_.clear();
    threads_.reserve(numThreads_);
    
    for (int i = 0; i < numThreads_; ++i) {
        threads_.emplace_back();
        threads_[i].bestFitness = std::numeric_limits<double>::max();
        threads_[i].thread = std::thread(&ParallelSimulatedAnnealing::workerThread, this, i);
    }
}

void ParallelSimulatedAnnealing::stop() {
    shouldStop_ = true;
    
    for (auto& threadData : threads_) {
        if (threadData.algorithm) {
            threadData.algorithm->stop();
        }
    }
    
    for (auto& threadData : threads_) {
        if (threadData.thread.joinable()) {
            threadData.thread.join();
        }
    }
}

bool ParallelSimulatedAnnealing::exchangeSolutions() {
    std::lock_guard<std::mutex> lock(exchangeMutex_);
    
    bool globalImproved = false;
    
    for (auto& threadData : threads_) {
        std::lock_guard<std::mutex> solutionLock(threadData.solutionMutex);
        
        if (threadData.bestSolution && threadData.bestFitness < globalBestFitness_) {
            globalBestFitness_ = threadData.bestFitness;
            globalBestSolution_ = threadData.bestSolution->clone();
            globalImproved = true;
        }
    }
    
    if (globalImproved) {
        for (auto& threadData : threads_) {
            if (threadData.algorithm && globalBestSolution_) {
                threadData.algorithm->setCurrentSolution(globalBestSolution_);
            }
        }
    }
    
    return globalImproved;
}

std::shared_ptr<ISolution> ParallelSimulatedAnnealing::createThreadSpecificSolution() {
    std::lock_guard<std::mutex> lock(initialSolutionMutex_);
    
    if (!initialSolutionTemplate_) {
        return nullptr;
    }
    
    auto solution = initialSolutionTemplate_->clone();
    
    return solution;
}