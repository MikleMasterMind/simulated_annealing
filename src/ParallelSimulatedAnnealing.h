#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "SimulatedAnnealing.h"
#include "ISolution.h"
#include "IMutation.h"
#include "ICoolingLaw.h"

class ParallelSimulatedAnnealing {
public:
    explicit ParallelSimulatedAnnealing(int numThreads);
    ~ParallelSimulatedAnnealing();
    
    void setInitialSolution(const std::shared_ptr<ISolution>& solution);
    void setMutation(const std::shared_ptr<IMutation>& mutation);
    void setCoolingLaw(const std::shared_ptr<ICoolingLaw>& coolingLaw);
    void setInitialTemperature(double temperature);
    void setIterationsPerTemperature(int iterations);
    void setMaxIterationsWithoutImprovement(int iterations);
    void setMaxIterationsWithoutImprovementGlobal(int iterations);
    void setExchangeInterval(int interval);
    
    std::shared_ptr<ISolution> run();
    void stop();

private:
    struct ThreadData {
        std::unique_ptr<SimulatedAnnealing> algorithm;
        std::shared_ptr<ISolution> bestSolution;
        double bestFitness;
        std::thread thread;
        std::mutex solutionMutex;
        
        // Конструкторы для поддержки копирования/перемещения
        ThreadData() : bestFitness(std::numeric_limits<double>::max()) {}
        ThreadData(const ThreadData&) = delete;
        ThreadData& operator=(const ThreadData&) = delete;
        ThreadData(ThreadData&& other) noexcept
            : algorithm(std::move(other.algorithm))
            , bestSolution(std::move(other.bestSolution))
            , bestFitness(other.bestFitness)
            , thread(std::move(other.thread)) {
        }
        ThreadData& operator=(ThreadData&& other) noexcept {
            if (this != &other) {
                algorithm = std::move(other.algorithm);
                bestSolution = std::move(other.bestSolution);
                bestFitness = other.bestFitness;
                thread = std::move(other.thread);
            }
            return *this;
        }
    };
    
    int numThreads_;
    int exchangeInterval_;
    std::atomic<bool> shouldStop_;
    std::atomic<int> iterationsWithoutImprovement_;
    
    std::shared_ptr<ISolution> globalBestSolution_;
    std::atomic<double> globalBestFitness_;
    
    std::vector<ThreadData> threads_;
    std::mutex exchangeMutex_;
    std::mutex initialSolutionMutex_;
    std::mutex configMutex_;
    
    std::shared_ptr<ISolution> initialSolutionTemplate_;
    std::shared_ptr<IMutation> mutation_;
    std::shared_ptr<ICoolingLaw> coolingLaw_;
    double initialTemperature_;
    int iterationsPerTemperature_;
    int maxIterationsWithoutImprovement_;
    int maxIterationsWithoutImprovementGlobal_;
    
    void workerThread(int threadId);
    void initializeThreads();
    bool exchangeSolutions();
    std::shared_ptr<ISolution> createThreadSpecificSolution();
};