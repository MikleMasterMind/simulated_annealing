#include "ParallelSimulatedAnnealing.h"
#include "Logger.h"
#include <chrono>
#include <random>
#include <algorithm>
#include <thread>

ParallelSimulatedAnnealing::ParallelSimulatedAnnealing(int numThreads)
    : numThreads_(numThreads)
    , exchangeInterval_(0)
    , shouldStop_(false)
    , iterationsWithoutImprovement_(0)
    , globalBestFitness_(std::numeric_limits<double>::max())
    , initialTemperature_(0.0)
    , iterationsPerTemperature_(0)
    , maxIterationsWithoutImprovement_(0) {
    
    if (numThreads_ <= 0) {
        numThreads_ = std::thread::hardware_concurrency();
        if (numThreads_ == 0) numThreads_ = 1;
    }
    
    Logger::log("ParallelSimulatedAnnealing created with " + std::to_string(numThreads_) + " threads");
}

ParallelSimulatedAnnealing::~ParallelSimulatedAnnealing() {
    stop();
}

void ParallelSimulatedAnnealing::setInitialSolution(const std::shared_ptr<ISolution>& solution) {
    initialSolutionTemplate_ = solution;
    Logger::log("Initial solution set for parallel algorithm");
}

void ParallelSimulatedAnnealing::setMutation(const std::shared_ptr<IMutation>& mutation) {
    mutation_ = mutation;
}

void ParallelSimulatedAnnealing::setCoolingLaw(const std::shared_ptr<ICoolingLaw>& coolingLaw) {
    coolingLaw_ = coolingLaw;
}

void ParallelSimulatedAnnealing::setInitialTemperature(double temperature) {
    initialTemperature_ = temperature;
    Logger::log("Parallel initial temperature set to: " + std::to_string(temperature));
}

void ParallelSimulatedAnnealing::setIterationsPerTemperature(int iterations) {
    iterationsPerTemperature_ = iterations;
    Logger::log("Parallel iterations per temperature set to: " + std::to_string(iterations));
}

void ParallelSimulatedAnnealing::setMaxIterationsWithoutImprovement(int iterations) {
    maxIterationsWithoutImprovement_ = iterations;
    Logger::log("Parallel max iterations without improvement set to: " + std::to_string(iterations));
}

void ParallelSimulatedAnnealing::setMaxIterationsWithoutImprovementGlobal(int iterations) {
    maxIterationsWithoutImprovementGlobal_ = iterations;
    Logger::log("Parallel max iterations without improvement global set to: " + std::to_string(iterations));
}

void ParallelSimulatedAnnealing::setExchangeInterval(int interval) {
    exchangeInterval_ = interval;
    Logger::log("Exchange interval set to: " + std::to_string(interval));
}

std::shared_ptr<ISolution> ParallelSimulatedAnnealing::run() {
    if (!initialSolutionTemplate_ || !mutation_ || !coolingLaw_) {
        Logger::log("ERROR: Parallel algorithm not properly initialized");
        return nullptr;
    }
    
    globalBestSolution_ = initialSolutionTemplate_->clone();
    globalBestFitness_ = globalBestSolution_->evaluate();
    
    iterationsWithoutImprovement_ = 0;
    shouldStop_ = false;
    
    Logger::log("Parallel algorithm STARTED: threads=" + std::to_string(numThreads_) +
                ", initial_fitness=" + std::to_string(globalBestFitness_) +
                ", exchange_interval=" + std::to_string(exchangeInterval_));
    
    initializeThreads();
    
    int globalCycle = 0;
    
    while (iterationsWithoutImprovement_ < maxIterationsWithoutImprovementGlobal_ && !shouldStop_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        bool improved = exchangeSolutions();
        
        if (improved) {
            iterationsWithoutImprovement_ = 0;
            Logger::log("Global improvement found in cycle " + std::to_string(globalCycle));
        } else {
            iterationsWithoutImprovement_++;
            Logger::log("No global improvement in cycle " + std::to_string(globalCycle) +
                        ", count=" + std::to_string(iterationsWithoutImprovement_));
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
                Logger::log("All threads finished, stopping parallel algorithm");
                break;
            }
        }
    }
    
    stop();
    
    exchangeSolutions();
    
    Logger::log("Parallel algorithm FINISHED: global_cycles=" + std::to_string(globalCycle) +
                ", final_fitness=" + std::to_string(globalBestFitness_) +
                ", total_improvement=" + std::to_string(initialSolutionTemplate_->evaluate() - globalBestFitness_));
    
    return globalBestSolution_;
}

void ParallelSimulatedAnnealing::workerThread(int threadId) {
    auto& threadData = threads_[threadId];
    
    Logger::log("Worker thread " + std::to_string(threadId) + " started");
    
    threadData.algorithm = std::make_unique<SimulatedAnnealing>();
    threadData.algorithm->setInitialSolution(createThreadSpecificSolution());
    threadData.algorithm->setMutation(mutation_);
    threadData.algorithm->setCoolingLaw(coolingLaw_);
    threadData.algorithm->setInitialTemperature(initialTemperature_);
    threadData.algorithm->setIterationsPerTemperature(iterationsPerTemperature_);
    threadData.algorithm->setMaxIterationsWithoutImprovement(maxIterationsWithoutImprovement_);
    
    int localIterations = 0;
    
    while (!shouldStop_ && localIterations < exchangeInterval_) {
        std::shared_ptr<ISolution> localBest = nullptr;
        {
            std::lock_guard<std::mutex> lock(threadData.solutionMutex);
            localBest = threadData.algorithm->run();
        }
        
        if (localBest) {
            std::lock_guard<std::mutex> lock(threadData.solutionMutex);

            double localFitness = localBest->evaluate();
            threadData.bestSolution = localBest;
            threadData.bestFitness = localFitness;
            
            Logger::log("Thread " + std::to_string(threadId) + " local best: " + std::to_string(localFitness));
        }
        
        ++localIterations;
        
        if (shouldStop_) {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    Logger::log("Worker thread " + std::to_string(threadId) + " finished");
}

void ParallelSimulatedAnnealing::initializeThreads() {
    threads_.clear();
    threads_.reserve(numThreads_);
    
    Logger::log("Initializing " + std::to_string(numThreads_) + " worker threads");
    
    for (int i = 0; i < numThreads_; ++i) {
        threads_.emplace_back();
        threads_[i].bestFitness = std::numeric_limits<double>::max();
        threads_[i].thread = std::thread(&ParallelSimulatedAnnealing::workerThread, this, i);
    }
    
    Logger::log("All worker threads started");
}

void ParallelSimulatedAnnealing::stop() {
    Logger::log("Stopping parallel algorithm and all worker threads");
    
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
    
    Logger::log("All worker threads stopped");
}

bool ParallelSimulatedAnnealing::exchangeSolutions() {
    bool globalImproved = false;
    double previousBest = globalBestFitness_;
    
    for (auto& threadData : threads_) {
        std::lock_guard<std::mutex> lock(threadData.solutionMutex);
        if (threadData.bestSolution && threadData.bestFitness < globalBestFitness_) {
            globalBestFitness_ = threadData.bestFitness;
            globalBestSolution_ = threadData.bestSolution->clone();
            globalImproved = true;
            std::cout << "Global improvment to : " << threadData.bestFitness << std::endl;
            
            Logger::log("GLOBAL IMPROVEMENT: thread " + 
                        std::to_string(&threadData - &threads_[0]) +
                        " improved fitness from " + std::to_string(previousBest) +
                        " to " + std::to_string(globalBestFitness_));
        }
    }
    
    if (globalImproved) {
        Logger::log("Broadcasting global best solution to all threads");
        for (auto& threadData : threads_) {
            if (threadData.algorithm && globalBestSolution_) {
                std::lock_guard<std::mutex> lock(threadData.solutionMutex);
                threadData.algorithm->setCurrentSolution(globalBestSolution_);
            }
        }
    } else {
        Logger::log("No global improvement in this exchange");
    }
    
    return globalImproved;
}

std::shared_ptr<ISolution> ParallelSimulatedAnnealing::createThreadSpecificSolution() {
    if (!initialSolutionTemplate_) {
        return nullptr;
    }
    
    auto solution = mutation_->apply(initialSolutionTemplate_->clone());
    
    return solution;
}