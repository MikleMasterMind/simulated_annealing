#include <iostream>
#include <memory>
#include <chrono>
#include <string>
#include "ParallelSimulatedAnnealing.h"
#include "ScheduleSolution.h"
#include "ScheduleMutation.h"
#include "SolutionGenerator.h"
#include "BoltzmannCooling.h"
#include "CauchyCooling.h"
#include "LogarithmicCooling.h"
#include "CSVDataGenerator.h"
#include "CSVDataReader.h"
#include "Logger.h"

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " <job_count>  <processor_count> <min_duration> <max_duration> <exchange_interval> <initial_temperature> <cooling_law> <iterations_per_temperature> <iterations_without_improvement> <iterations_without_improvement_global> <num_threads>" << std::endl;
    std::cout << "Example: " << programName << " 10 2 1.0 15.0 100 1000.0 boltzmann 50 1000 10 4" << std::endl;
    std::cout << "Cooling laws: boltzmann, cauchy, logarithmic" << std::endl;
    std::cout << "Default exchange_interval: 100" << std::endl;
}

std::shared_ptr<ICoolingLaw> createCoolingLaw(const std::string& lawName) {
    if (lawName == "boltzmann") {
        return std::make_shared<BoltzmannCooling>();
    } else if (lawName == "cauchy") {
        return std::make_shared<CauchyCooling>();
    } else if (lawName == "logarithmic") {
        return std::make_shared<LogarithmicCooling>();
    } else {
        throw std::invalid_argument("Unknown cooling law: " + lawName);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 8) {
        std::cerr << "Error: Invalid number of arguments" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    bool enableLogging = true; // или false чтобы отключить
    Logger::initialize(enableLogging, "simulated_annealing.log");

    try {
        int jobCount = std::stoi(argv[1]);
        int processorCount = std::stoi(argv[2]);
        double jobMinDuration = std::stod(argv[3]);
        double jobMaxDuration = std::stod(argv[4]);
        int exchangeInterval = std::stoi(argv[5]);
        double initialTemperature = std::stod(argv[6]);
        std::string coolingLawName = argv[7];
        int iterationsPerTemperature = std::stoi(argv[8]);
        int iterationsWithoutImprovement = std::stoi(argv[9]);
        int iterationsWithoutImprovementGlobal = std::stoi(argv[10]);
        int numThreads = std::stoi(argv[11]);

        if (jobCount <= 0 || processorCount <= 0 || numThreads <= 0 || exchangeInterval <= 0) {
            throw std::invalid_argument("All numeric parameters must be positive");
        }
        if (jobMinDuration <= 0 || jobMaxDuration <= jobMinDuration) {
            throw std::invalid_argument("Invalid duration range");
        }
        if (initialTemperature <= 0) {
            throw std::invalid_argument("Initial temperature must be positive");
        }

        auto coolingLaw = createCoolingLaw(coolingLawName);

        std::cout << "=== Parallel Simulated Annealing Scheduler ===" << std::endl;
        std::cout << "Parameters: " << jobCount << " jobs, " << processorCount 
                  << " processors, duration range [" << jobMinDuration << ", " << jobMaxDuration << "]" << std::endl;
        std::cout << "Algorithm: " << numThreads << " threads, " << coolingLawName 
                  << " cooling, T0=" << initialTemperature << std::endl;
        std::cout << "Exchange interval: " << exchangeInterval << std::endl;
        
        CSVDataGenerator dataGenerator;
        dataGenerator.generateData(jobCount, processorCount, jobMinDuration, jobMaxDuration, "input.csv");
        
        CSVDataReader reader;
        InputData data = reader.readData("input.csv");
        
        std::cout << "\n1. Creating initial solution..." << std::endl;
        SolutionGenerator generator;
        auto initialSolution = generator.generateWorstCaseSolution(data.jobCount, data.processorCount, data.jobDurations);
        double initialFitness = initialSolution->evaluate();
        std::cout << "Initial solution fitness: " << initialFitness << std::endl;
        
        std::cout << "\n2. Configuring parallel simulated annealing..." << std::endl;
        auto mutation = std::make_shared<ScheduleMutation>();
        coolingLaw->initialize(initialTemperature);
        
        ParallelSimulatedAnnealing psa(numThreads);
        psa.setInitialSolution(initialSolution);
        psa.setMutation(mutation);
        psa.setCoolingLaw(coolingLaw);
        psa.setInitialTemperature(initialTemperature);
        psa.setIterationsPerTemperature(iterationsPerTemperature);
        psa.setMaxIterationsWithoutImprovement(iterationsWithoutImprovement);
        psa.setMaxIterationsWithoutImprovementGlobal(iterationsWithoutImprovementGlobal);
        psa.setExchangeInterval(exchangeInterval);
        
        std::cout << "Initial temperature: " << initialTemperature << std::endl;
        std::cout << "Cooling law: " << coolingLawName << std::endl;
        std::cout << "Iterations per temperature: " << iterationsPerTemperature << std::endl;
        std::cout << "Max iterations without improvement: " << iterationsWithoutImprovement << std::endl;
        std::cout << "Exchange interval: " << exchangeInterval << std::endl;
        std::cout << "Number of threads: " << numThreads << std::endl;
        
        std::cout << "\n3. Running parallel simulated annealing..." << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        auto bestSolution = psa.run();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Algorithm completed in " << duration.count() << " ms" << std::endl;
        
        std::cout << "\n4. Results:" << std::endl;
        if (bestSolution) {
            auto scheduleSolution = std::dynamic_pointer_cast<ScheduleSolution>(bestSolution);
            if (scheduleSolution) {
                double bestFitness = scheduleSolution->evaluate();
                std::cout << "Best solution fitness: " << bestFitness << std::endl;
                std::cout << "Improvement: " << (initialFitness - bestFitness) << std::endl;
                std::cout << "Improvement percentage: " << ((initialFitness - bestFitness) / initialFitness * 100) << "%" << std::endl;
                
                // std::cout << "\nFinal schedule:" << std::endl;
                // for (int j = 0; j < processorCount; ++j) {
                //     std::cout << "Processor " << j << ": ";
                //     double processorTime = 0.0;
                //     for (int i = 0; i < jobCount; ++i) {
                //         if (scheduleSolution->isJobAssignedToProcessor(i, j)) {
                //             std::cout << "J" << i << "(" << data.jobDurations[i] << ") ";
                //             processorTime += data.jobDurations[i];
                //         }
                //     }
                //     std::cout << "→ Total: " << processorTime << std::endl;
                // }
            }
        } else {
            std::cout << "No solution found!" << std::endl;
        }
        
        std::cout << "\n5. Validating solution..." << std::endl;
        if (bestSolution) {
            auto scheduleSolution = std::dynamic_pointer_cast<ScheduleSolution>(bestSolution);
            if (scheduleSolution) {
                bool isValid = true;
                for (int i = 0; i < jobCount; ++i) {
                    int assignmentCount = 0;
                    for (int j = 0; j < processorCount; ++j) {
                        if (scheduleSolution->isJobAssignedToProcessor(i, j)) {
                            assignmentCount++;
                        }
                    }
                    if (assignmentCount != 1) {
                        isValid = false;
                        std::cout << "ERROR: Job " << i << " assigned to " << assignmentCount << " processors!" << std::endl;
                    }
                }
                
                if (isValid) {
                    std::cout << "Solution is valid - all jobs assigned correctly" << std::endl;
                } else {
                    std::cout << "Solution is invalid!" << std::endl;
                }
                
                std::vector<bool> jobsAssigned(jobCount, false);
                for (int i = 0; i < jobCount; ++i) {
                    for (int j = 0; j < processorCount; ++j) {
                        if (scheduleSolution->isJobAssignedToProcessor(i, j)) {
                            jobsAssigned[i] = true;
                            break;
                        }
                    }
                }
                
                bool allJobsAssigned = true;
                for (int i = 0; i < jobCount; ++i) {
                    if (!jobsAssigned[i]) {
                        allJobsAssigned = false;
                        std::cout << "ERROR: Job " << i << " is not assigned to any processor!" << std::endl;
                    }
                }
                
                if (allJobsAssigned) {
                    std::cout << "All jobs are assigned to processors" << std::endl;
                } else {
                    std::cout << "Some jobs are missing!" << std::endl;
                }
            }
        }
        
        std::cout << "\n=== Parallel algorithm finished ===" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        printUsage(argv[0]);
        return 1;
    }
}